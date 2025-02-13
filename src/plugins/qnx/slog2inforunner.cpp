// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "slog2inforunner.h"

#include "qnxtr.h"

#include <projectexplorer/devicesupport/idevice.h>
#include <projectexplorer/runconfigurationaspects.h>

#include <utils/qtcprocess.h>
#include <utils/qtcassert.h>

#include <QRegularExpression>

using namespace ProjectExplorer;
using namespace Utils;

namespace Qnx::Internal {

Slog2InfoRunner::Slog2InfoRunner(RunControl *runControl)
    : RunWorker(runControl)
{
    setId("Slog2InfoRunner");
    m_applicationId = runControl->aspectData<ExecutableAspect>()->executable.fileName();

    // See QTCREATORBUG-10712 for details.
    // We need to limit length of ApplicationId to 63 otherwise it would not match one in slog2info.
    m_applicationId.truncate(63);
}

void Slog2InfoRunner::start()
{
    using namespace Tasking;
    QTC_CHECK(!m_taskTreeRunner.isRunning());

    const auto onTestSetup = [this](Process &process) {
        process.setCommand(CommandLine{runControl()->device()->filePath("slog2info")});
    };
    const auto onTestDone = [this] {
        appendMessage(Tr::tr("Warning: \"slog2info\" is not found on the device, "
                             "debug output not available."), ErrorMessageFormat);
    };

    const auto onLaunchTimeSetup = [this](Process &process) {
        process.setCommand({runControl()->device()->filePath("date"), "+\"%d %H:%M:%S\"", CommandLine::Raw});
    };
    const auto onLaunchTimeDone = [this](const Process &process) {
        QTC_CHECK(!m_applicationId.isEmpty());
        m_launchDateTime = QDateTime::fromString(process.cleanedStdOut().trimmed(), "dd HH:mm:ss");
    };

    const auto onLogSetup = [this](Process &process) {
        process.setCommand({runControl()->device()->filePath("slog2info"), {"-w"}});
        connect(&process, &Process::readyReadStandardOutput, this, [this, processPtr = &process] {
            processLogInput(QString::fromLatin1(processPtr->readAllRawStandardOutput()));
        });
        connect(&process, &Process::readyReadStandardError, this, [this, processPtr = &process] {
            appendMessage(QString::fromLatin1(processPtr->readAllRawStandardError()), StdErrFormat);
        });
    };
    const auto onLogError = [this](const Process &process) {
        appendMessage(Tr::tr("Cannot show slog2info output. Error: %1").arg(process.errorString()),
                      StdErrFormat);
    };

    const Group root {
        ProcessTask(onTestSetup, onTestDone, CallDoneIf::Error),
        ProcessTask(onLaunchTimeSetup, onLaunchTimeDone, CallDoneIf::Success),
        ProcessTask(onLogSetup, onLogError, CallDoneIf::Error)
    };

    m_taskTreeRunner.start(root);
    reportStarted();
}

void Slog2InfoRunner::stop()
{
    m_taskTreeRunner.reset();
    processRemainingLogData();
    reportStopped();
}

void Slog2InfoRunner::processRemainingLogData()
{
    if (!m_remainingData.isEmpty())
        processLogLine(m_remainingData);
    m_remainingData.clear();
}

void Slog2InfoRunner::processLogInput(const QString &input)
{
    QStringList lines = input.split(QLatin1Char('\n'));
    if (lines.isEmpty())
        return;
    lines.first().prepend(m_remainingData);
    m_remainingData = lines.takeLast();
    for (const QString &line : std::as_const(lines))
        processLogLine(line);
}

void Slog2InfoRunner::processLogLine(const QString &line)
{
    // The "(\\s+\\S+)?" represents a named buffer. If message has noname (aka empty) buffer
    // then the message might get cut for the first number in the message.
    // The "\\s+(\\b.*)?$" represents a space followed by a message. We are unable to determinate
    // how many spaces represent separators and how many are a part of the messages, so resulting
    // messages has all whitespaces at the beginning of the message trimmed.
    static const QRegularExpression regexp(
        "^[a-zA-Z]+\\s+([0-9]+ [0-9]+:[0-9]+:[0-9]+.[0-9]+)\\s+(\\S+)(\\s+(\\S+))?\\s+([0-9]+)\\s+(.*)?$");

    const QRegularExpressionMatch match = regexp.match(line);
    if (!match.hasMatch())
        return;

    // Note: This is useless if/once slog2info -b displays only logs from recent launches
    if (!m_launchDateTime.isNull()) {
        // Check if logs are from the recent launch
        if (!m_currentLogs) {
            QDateTime dateTime = QDateTime::fromString(match.captured(1),
                                                       QLatin1String("dd HH:mm:ss.zzz"));
            m_currentLogs = dateTime >= m_launchDateTime;
            if (!m_currentLogs)
                return;
        }
    }

    QString applicationId = match.captured(2);
    if (!applicationId.startsWith(m_applicationId))
        return;

    QString bufferName = match.captured(4);
    int bufferId = match.captured(5).toInt();
    // filtering out standard BB10 messages
    if (bufferName == QLatin1String("default") && bufferId == 8900)
        return;

    appendMessage(match.captured(6).trimmed() + '\n', StdOutFormat);
}

} // Qnx::Internal
