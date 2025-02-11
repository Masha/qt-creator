// Copyright (c) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "haskellconstants.h"
#include "haskelleditorfactory.h"
#include "haskellmanager.h"
#include "haskellproject.h"
#include "haskellrunconfiguration.h"
#include "haskelltr.h"
#include "stackbuildstep.h"

#include <extensionsystem/iplugin.h>

#include <coreplugin/dialogs/ioptionspage.h>

#include <texteditor/snippets/snippetprovider.h>

namespace Haskell::Internal {

class HaskellPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Haskell.json")

public:
    void initialize() final
    {
        Core::IOptionsPage::registerCategory(
            "J.Z.Haskell", Tr::tr("Haskell"), ":/haskell/images/settingscategory_haskell.png");

        setupHaskellStackBuildStep();

        setupHaskellRunSupport();

        setupHaskellEditor();

        setupHaskellProject();

        TextEditor::SnippetProvider::registerGroup(Constants::C_HASKELLSNIPPETSGROUP_ID,
                                                   Tr::tr("Haskell", "SnippetProvider"));

        setupHaskellActions(this);
    }
};

} // Haskell::Internal

#include "haskellplugin.moc"
