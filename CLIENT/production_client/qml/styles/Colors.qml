pragma Singleton
import QtQuick 6.0

QtObject {
    property bool isDarkTheme: true
    
    // Тёмная тема
    property color darkBackground: "#121212"
    property color darkSurface: "#1E1E1E"
    property color darkPrimary: "#2C3E50"
    property color darkText: "#FFFFFF"
    property color darkTextSecondary: "#B0B0B0"
    property color darkBorder: "#404040"
    property color darkButton: "#0D47A1"
    property color darkButtonHover: "#1565C0"
    property color darkButtonPressed: "#0A3A8A"
    property color darkError: "#FF5252"
    property color darkWarning: "#FFC107"
    property color darkSuccess: "#4CAF50"
    
    // Светлая тема
    property color lightBackground: "#F5F5F5"
    property color lightSurface: "#FFFFFF"
    property color lightPrimary: "#E3F2FD"
    property color lightText: "#212121"
    property color lightTextSecondary: "#757575"
    property color lightBorder: "#CCCCCC"
    property color lightButton: "#2196F3"
    property color lightButtonHover: "#64B5F6"
    property color lightButtonPressed: "#1976D2"
    property color lightError: "#F44336"
    property color lightWarning: "#FF9800"
    property color lightSuccess: "#4CAF50"
    
    // Текущие цвета
    property color background: darkBackground
    property color surface: darkSurface
    property color primary: darkPrimary
    property color text: darkText
    property color textSecondary: darkTextSecondary
    property color border: darkBorder
    property color button: darkButton
    property color buttonHover: darkButtonHover
    property color buttonPressed: darkButtonPressed
    property color error: darkError
    property color warning: darkWarning
    property color success: darkSuccess
    property color buttonText: "#FFFFFF"
    
    function setDarkTheme() {
        isDarkTheme = true
        background = darkBackground
        surface = darkSurface
        primary = darkPrimary
        text = darkText
        textSecondary = darkTextSecondary
        border = darkBorder
        button = darkButton
        buttonHover = darkButtonHover
        buttonPressed = darkButtonPressed
        error = darkError
        warning = darkWarning
        success = darkSuccess
    }
    
    function setLightTheme() {
        isDarkTheme = false
        background = lightBackground
        surface = lightSurface
        primary = lightPrimary
        text = lightText
        textSecondary = lightTextSecondary
        border = lightBorder
        button = lightButton
        buttonHover = lightButtonHover
        buttonPressed = lightButtonPressed
        error = lightError
        warning = lightWarning
        success = lightSuccess
    }
}
