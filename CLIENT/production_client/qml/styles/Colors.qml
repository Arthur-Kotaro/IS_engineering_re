pragma Singleton
import QtQuick 6.0

QtObject {
    property bool isDarkTheme: true
    property int lightScheme: 0
    property int darkScheme: 0

    property color darkBg0: "#121212"
    property color darkSurface0: "#1E1E1E"
    property color darkPrimary0: "#2C3E50"
    property color darkText0: "#FFFFFF"
    property color darkTextSec0: "#B0B0B0"
    property color darkBorder0: "#404040"
    property color darkButton0: "#0D47A1"
    property color darkButtonHover0: "#1565C0"
    property color darkError0: "#FF5252"
    property color darkWarning0: "#FFC107"
    property color darkSuccess0: "#4CAF50"

    property color darkBg1: "#1A1A1A"
    property color darkSurface1: "#2A2A2A"
    property color darkPrimary1: "#1B5E20"
    property color darkText1: "#E8F5E9"
    property color darkTextSec1: "#A5D6A7"
    property color darkBorder1: "#4CAF50"
    property color darkButton1: "#2E7D32"
    property color darkButtonHover1: "#388E3C"
    property color darkError1: "#EF5350"
    property color darkWarning1: "#FFD54F"
    property color darkSuccess1: "#66BB6A"

    property color darkBg2: "#1A1A2E"
    property color darkSurface2: "#2D2D44"
    property color darkPrimary2: "#4A148C"
    property color darkText2: "#F3E5F5"
    property color darkTextSec2: "#CE93D8"
    property color darkBorder2: "#7B1FA2"
    property color darkButton2: "#6A1B9A"
    property color darkButtonHover2: "#8E24AA"
    property color darkError2: "#EF5350"
    property color darkWarning2: "#FFD54F"
    property color darkSuccess2: "#66BB6A"

    property color lightBg0: "#F5F5F5"
    property color lightSurface0: "#FFFFFF"
    property color lightPrimary0: "#E3F2FD"
    property color lightText0: "#212121"
    property color lightTextSec0: "#757575"
    property color lightBorder0: "#CCCCCC"
    property color lightButton0: "#2196F3"
    property color lightButtonHover0: "#64B5F6"
    property color lightError0: "#F44336"
    property color lightWarning0: "#FF9800"
    property color lightSuccess0: "#4CAF50"

    property color lightBg1: "#F1F8E9"
    property color lightSurface1: "#FFFFFF"
    property color lightPrimary1: "#C8E6C9"
    property color lightText1: "#1B5E20"
    property color lightTextSec1: "#388E3C"
    property color lightBorder1: "#81C784"
    property color lightButton1: "#43A047"
    property color lightButtonHover1: "#66BB6A"
    property color lightError1: "#E53935"
    property color lightWarning1: "#FB8C00"
    property color lightSuccess1: "#388E3C"

    property color lightBg2: "#F3E5F5"
    property color lightSurface2: "#FFFFFF"
    property color lightPrimary2: "#E1BEE7"
    property color lightText2: "#4A148C"
    property color lightTextSec2: "#7B1FA2"
    property color lightBorder2: "#AB47BC"
    property color lightButton2: "#8E24AA"
    property color lightButtonHover2: "#AB47BC"
    property color lightError2: "#E53935"
    property color lightWarning2: "#FB8C00"
    property color lightSuccess2: "#388E3C"

    property color background: darkBg0
    property color surface: darkSurface0
    property color primary: darkPrimary0
    property color text: darkText0
    property color textSecondary: darkTextSec0
    property color border: darkBorder0
    property color button: darkButton0
    property color buttonHover: darkButtonHover0
    property color error: darkError0
    property color warning: darkWarning0
    property color success: darkSuccess0
    property color buttonText: "#FFFFFF"

    function setDarkTheme() {
        isDarkTheme = true
        updateColors()
    }

    function setLightTheme() {
        isDarkTheme = false
        updateColors()
    }

    function toggleTheme() {
        isDarkTheme = !isDarkTheme
        updateColors()
    }

    function setLightScheme(index) {
        lightScheme = index
        if (!isDarkTheme) {
            updateColors()
        }
    }

    function setDarkScheme(index) {
        darkScheme = index
        if (isDarkTheme) {
            updateColors()
        }
    }

    function updateColors() {
        if (isDarkTheme) {
            if (darkScheme === 0) {
                background = darkBg0
                surface = darkSurface0
                primary = darkPrimary0
                text = darkText0
                textSecondary = darkTextSec0
                border = darkBorder0
                button = darkButton0
                buttonHover = darkButtonHover0
                error = darkError0
                warning = darkWarning0
                success = darkSuccess0
            } else if (darkScheme === 1) {
                background = darkBg1
                surface = darkSurface1
                primary = darkPrimary1
                text = darkText1
                textSecondary = darkTextSec1
                border = darkBorder1
                button = darkButton1
                buttonHover = darkButtonHover1
                error = darkError1
                warning = darkWarning1
                success = darkSuccess1
            } else {
                background = darkBg2
                surface = darkSurface2
                primary = darkPrimary2
                text = darkText2
                textSecondary = darkTextSec2
                border = darkBorder2
                button = darkButton2
                buttonHover = darkButtonHover2
                error = darkError2
                warning = darkWarning2
                success = darkSuccess2
            }
        } else {
            if (lightScheme === 0) {
                background = lightBg0
                surface = lightSurface0
                primary = lightPrimary0
                text = lightText0
                textSecondary = lightTextSec0
                border = lightBorder0
                button = lightButton0
                buttonHover = lightButtonHover0
                error = lightError0
                warning = lightWarning0
                success = lightSuccess0
            } else if (lightScheme === 1) {
                background = lightBg1
                surface = lightSurface1
                primary = lightPrimary1
                text = lightText1
                textSecondary = lightTextSec1
                border = lightBorder1
                button = lightButton1
                buttonHover = lightButtonHover1
                error = lightError1
                warning = lightWarning1
                success = lightSuccess1
            } else {
                background = lightBg2
                surface = lightSurface2
                primary = lightPrimary2
                text = lightText2
                textSecondary = lightTextSec2
                border = lightBorder2
                button = lightButton2
                buttonHover = lightButtonHover2
                error = lightError2
                warning = lightWarning2
                success = lightSuccess2
            }
        }
        buttonText = "#FFFFFF"
    }
}
