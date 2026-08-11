pragma Singleton
import QtQuick 6.0

QtObject {
    function capitalize(str) {
        return str.charAt(0).toUpperCase() + str.slice(1)
    }
}
