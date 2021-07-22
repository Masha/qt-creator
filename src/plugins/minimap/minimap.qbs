import qbs 1.0

QtcPlugin {
    name: "Minimap"

    Depends { name: "Qt"; submodules: ["widgets"] }

    Depends { name: "Utils" }
    Depends { name: "Core" }
    Depends { name: "TextEditor" }

    property bool enable: false

    files: [
        "minimapconstants.h",
        "minimap_global.h",
        "minimapsettings.h",
        "minimapstyle.h",
        "minimapsettings.cpp",
        "minimapstyle.cpp",
        "minimap.h",
        "minimap.cpp",
    ]
}
