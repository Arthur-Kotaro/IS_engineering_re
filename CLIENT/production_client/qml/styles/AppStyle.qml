pragma Singleton
import QtQuick 6.0

QtObject {
    function initialize() {
        console.log("AppStyle initialized with high contrast theme")
    }
    
    // Размеры отступов
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
    
    // Радиусы скругления
    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    
    // Тени
    readonly property int elevationSmall: 2
    readonly property int elevationMedium: 4
    readonly property int elevationLarge: 8
}
