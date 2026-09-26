pragma Singleton
import QtQuick 6.0

QtObject {
    property int fontSize: 14
    
    signal fontSizeUpdated()
    
    function setFontSize(value) {
        if (fontSize !== value) {
            fontSize = value
            fontSizeUpdated()
        }
    }
}
