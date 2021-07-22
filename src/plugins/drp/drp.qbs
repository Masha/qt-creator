import qbs 1.0

QtcPlugin {
    name: "drp"

    Depends { name: "Qt"; submodules: ["widgets", "xml", "network", "qml"] }
    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }

    property bool enable: false

    files: [
        "qtcreatordiscordrichpresenceplugin.h",
        "qtcreatordiscordrichpresence_global.h",
        "qtcreatordiscordrichpresenceconstants.h",
        "qtcreatordiscordrichpresenceplugin.cpp",
    ]
}
