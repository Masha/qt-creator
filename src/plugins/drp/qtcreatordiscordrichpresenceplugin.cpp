#include "qtcreatordiscordrichpresenceplugin.h"
#include "qtcreatordiscordrichpresenceconstants.h"

#include <time.h>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projecttree.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QDebug>

namespace QtCreatorDiscordRichPresence {
    namespace Internal {

        QtCreatorDiscordRichPresencePlugin::QtCreatorDiscordRichPresencePlugin()
        {
            // Create your members
        }

        QtCreatorDiscordRichPresencePlugin::~QtCreatorDiscordRichPresencePlugin()
        {
            // Unregister objects from the plugin manager's object pool
            // Delete members
        }

        bool QtCreatorDiscordRichPresencePlugin::initialize(const QStringList &arguments, QString *errorString)
        {
            // Register objects in the plugin manager's object pool
            // Load settings
            // Add actions to menus
            // Connect to other plugins' signals
            // In the initialize function, a plugin can be sure that the plugins it
            // depends on have initialized their members.

            Q_UNUSED(arguments)
            Q_UNUSED(errorString)

            //Register Discord Rich Presence
            initDiscord();
            DiscordRichPresence presence;
            memset(&presence, 0, sizeof(presence));
            presence.state = "Idle";
            presence.details = "Startup Screen";
            presence.largeImageKey = "logo3";
            presence.largeImageText = "near";
            presence.instance = 1;
            Discord_UpdatePresence(&presence);

            //Connect to file editing signals
            connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged, [=](Core::IEditor *editor) {

                DiscordRichPresence presence;
                memset(&presence, 0, sizeof(presence));

                if (editor == nullptr) return;

                //Depending on file extension choose an icon
                QString mimeType = editor->document()->mimeType();
                qDebug() << mimeType;

                const char *smallImageKey = "", *smallImageText;
                if (mimeType == "text/x-c++hdr") { //C++ Header
                    smallImageKey = "cpph";
                    smallImageText = "C++ Header";
                } else if (mimeType == "text/x-c++src") { //C++ Source
                    smallImageKey = "cpp";
                    smallImageText = "C++ Source";
                } else if (mimeType == "text/x-chdr") { //C Header
                    smallImageKey = "c";
                    smallImageText = "C Header";
                } else if (mimeType == "text/x-csrc") { //C Source
                    smallImageKey = "c";
                    smallImageText = "C Source";
                } else if (mimeType == "application/vnd.qt.qmakeprofile" || mimeType == "application/vnd.qt.qmakeproincludefile") { //QMake Profile
                    smallImageKey = "qt-logo";
                    smallImageText = "QMake Project Profile";
                } else if (mimeType == "application/x-designer") { //Designer
                    smallImageKey = "qt-logo";
                    smallImageText = "Qt User Interface File";
                } else if (mimeType == "application/vnd.qt.xml.resource") { //Qt Resource
                    smallImageKey = "qt-logo";
                    smallImageText = "Qt Resource File";
                } else if (mimeType == "application/x-desktop") { //Desktop File
                    smallImageKey = "qt-logo";
                    smallImageText = "Desktop File";
                } else if (mimeType == "application/json") { //JSON File
                    smallImageKey = "json";
                    smallImageText = "JSON File";
                } else if (mimeType == "text/plain") { //Text File
                    smallImageKey = "manifest";
                    smallImageText = "Plain Text";
                } else if (mimeType == "application/xml") { //XML File
                    smallImageKey = "xml";
                    smallImageText = "XML";
                } else if (mimeType == "text/html") { //HTML File
                    smallImageKey = "html";
                    smallImageText = "HTML";
                } else if (mimeType == "text/vnd.qtcreator.git.submit") { //Git commit window

                }

                if (smallImageKey != nullptr && smallImageKey[0] == '\0') {
                    presence.largeImageKey = "logo2";
                    presence.largeImageText = "ghoul";
                } else {
                    presence.largeImageKey = "logo3";
                    presence.largeImageText = "ghoul";
                    presence.smallImageKey = smallImageKey;
                    presence.smallImageText = smallImageText;
                }

                ProjectExplorer::Project* current = ProjectExplorer::ProjectTree::currentProject();
                char stateString[256];
                if (current == nullptr) {
                    if (editor->isDesignModePreferred()) {
                        sprintf(stateString, "Designing UI");
                    } else {
                        sprintf(stateString, "Editing");
                    }
                } else {
                    sprintf(stateString, "%s", current->displayName().prepend("Doing ").toUtf8().data());
                }
                presence.state = stateString;

                char detailsString[256];
                sprintf(detailsString, "%s", editor->document()->filePath().fileName().toUtf8().data());
                presence.details = detailsString;

                static int64_t StartTime = time(0);
                presence.startTimestamp = StartTime;

                Discord_UpdatePresence(&presence);
            });

            return true;
        }

        void QtCreatorDiscordRichPresencePlugin::extensionsInitialized()
        {
            // Retrieve objects from the plugin manager's object pool
            // In the extensionsInitialized function, a plugin can be sure that all
            // plugins that depend on it are completely initialized.
        }

        ExtensionSystem::IPlugin::ShutdownFlag QtCreatorDiscordRichPresencePlugin::aboutToShutdown()
        {
            // Save settings
            // Disconnect from signals that are not needed during shutdown
            // Hide UI (if you add UI that is not in the main window directly)

            Discord_Shutdown();
            return SynchronousShutdown;
        }
        void QtCreatorDiscordRichPresencePlugin::initDiscord() {
            DiscordEventHandlers handlers;
            memset(&handlers, 0, sizeof(handlers));
            handlers.ready = [](const DiscordUser* user) {
                qDebug() << "Discord Ready!";
            };
            handlers.errored = [](int errorCode, const char* message) {
                qDebug() << "Discord Error!";
            };
            handlers.disconnected = [](int errorCode, const char* message) {
                qDebug() << "Discord Disconnected!";
            };
            handlers.joinGame = [](const char* joinSecret) {
                QByteArray secretBytes(joinSecret);
                QString secret = QString::fromStdString(secretBytes.toStdString());
                QMessageBox::warning(nullptr, "Discord Join", QString("Discord Join Secret: ").append(secret), QMessageBox::Ok, QMessageBox::Ok);
            };
            Discord_Initialize("867759960355635242", &handlers, true, nullptr);
        }

    } // namespace Internal
} // namespace QtCreatorDiscordRichPresence
