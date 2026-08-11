pragma Singleton
import QtQuick 6.0

QtObject {
    function isValidEmail(email) {
        return email.includes('@')
    }
}
