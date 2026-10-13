pragma Singleton
import QtQuick 6.0

QtObject {
    // ============================================================
    // Универсальный диспетчер.
    // Возвращает: { valid: bool, message: string }
    // ============================================================
    function validate(type, text, required) {
        var t = text === undefined || text === null ? "" : String(text)
        var trimmed = t.trim()

        if (trimmed === "") {
            if (required) {
                return { valid: false, message: "Поле обязательно для заполнения" }
            }
            return { valid: true, message: "" }
        }

        if (type === "email")        return validateEmail(trimmed)
        if (type === "name")         return validateName(trimmed)
        if (type === "phone_mobile") return validatePhoneMobile(trimmed)
        if (type === "phone_work")   return validatePhoneWork(trimmed)
        if (type === "login")        return validateLogin(trimmed)
        if (type === "password")     return validatePassword(trimmed)

        return { valid: true, message: "" }
    }

    // ============================================================
    // Email.
    // Проверка вручную, без регулярок — QML V4 не поддерживает
    // квантификаторы {n,} и \uXXXX надёжно.
    // ============================================================
    function validateEmail(text) {
        // 1. Ровно один @
        var atCount = 0
        for (var i = 0; i < text.length; i++) {
            if (text.charCodeAt(i) === 0x40) atCount++
        }
        if (atCount !== 1) {
            return { valid: false, message: "Некорректный email" }
        }

        var atIdx = text.indexOf("@")
        var local = text.substring(0, atIdx)
        var domain = text.substring(atIdx + 1)

        if (local.length === 0 || domain.length < 3) {
            return { valid: false, message: "Некорректный email" }
        }

        // 2. Local part: A-Za-z0-9._%+-
        for (var j = 0; j < local.length; j++) {
            var c = local.charCodeAt(j)
            var okLocal =
                (c >= 0x41 && c <= 0x5A) ||   // A-Z
                (c >= 0x61 && c <= 0x7A) ||   // a-z
                (c >= 0x30 && c <= 0x39) ||   // 0-9
                c === 0x2E ||                 // .
                c === 0x5F ||                 // _
                c === 0x25 ||                 // %
                c === 0x2B ||                 // +
                c === 0x2D                    // -
            if (!okLocal) {
                return { valid: false, message: "Некорректный email" }
            }
        }

        // 3. Домен: A-Za-z0-9.-, хотя бы одна точка
        var hasDot = false
        for (var k = 0; k < domain.length; k++) {
            var d = domain.charCodeAt(k)
            var okDomain =
                (d >= 0x41 && d <= 0x5A) ||
                (d >= 0x61 && d <= 0x7A) ||
                (d >= 0x30 && d <= 0x39) ||
                d === 0x2E ||
                d === 0x2D
            if (!okDomain) {
                return { valid: false, message: "Некорректный email" }
            }
            if (d === 0x2E) hasDot = true
        }
        if (!hasDot) {
            return { valid: false, message: "Некорректный email" }
        }

        // 4. TLD (часть после последней точки) — только буквы, >= 2
        var lastDot = domain.lastIndexOf(".")
        var tld = domain.substring(lastDot + 1)
        if (tld.length < 2) {
            return { valid: false, message: "Некорректный email" }
        }
        for (var m = 0; m < tld.length; m++) {
            var t = tld.charCodeAt(m)
            if (!((t >= 0x41 && t <= 0x5A) || (t >= 0x61 && t <= 0x7A))) {
                return { valid: false, message: "Некорректный email" }
            }
        }

        // 5. Точка не первая и не последняя в домене
        if (domain.charAt(0) === "." || domain.charAt(domain.length - 1) === ".") {
            return { valid: false, message: "Некорректный email" }
        }

        return { valid: true, message: "" }
    }

    // ============================================================
    // ФИО.
    // Посимвольно через charCodeAt — QML V4 не поддерживает
    // \uXXXX в регулярках.
    // ============================================================
    function validateName(text) {
        for (var i = 0; i < text.length; i++) {
            var code = text.charCodeAt(i)
            var ok =
                (code >= 0x0400 && code <= 0x04FF) ||
                (code >= 0x0041 && code <= 0x005A) ||
                (code >= 0x0061 && code <= 0x007A) ||
                (code >= 0x4E00 && code <= 0x9FFF) ||
                code === 0x002D ||
                code === 0x0027
            if (!ok) {
                return { valid: false, message: "Допустимы только буквы (кириллица, латиница, китайские), дефис и апостроф" }
            }
        }
        return { valid: true, message: "" }
    }

    // ============================================================
    // Мобильный телефон.
    // Посимвольно через charCodeAt.
    // ============================================================
    function validatePhoneMobile(text) {
        var digitCount = 0
        for (var i = 0; i < text.length; i++) {
            var code = text.charCodeAt(i)
            var ok =
                (code >= 0x30 && code <= 0x39) ||
                code === 0x20 ||
                code === 0x2B ||
                code === 0x2D ||
                code === 0x28 ||
                code === 0x29
            if (!ok) {
                return { valid: false, message: "Некорректный номер. Допустимы цифры, +, -, (, ), пробел" }
            }
            if (code >= 0x30 && code <= 0x39) {
                digitCount++
            }
        }
        if (digitCount < 10 || digitCount > 15) {
            return { valid: false, message: "Номер должен содержать 10–15 цифр" }
        }
        return { valid: true, message: "" }
    }

    // ============================================================
    // Рабочий телефон.
    // ============================================================
    function validatePhoneWork(text) {
        return validatePhoneMobile(text)
    }

    // ============================================================
    // Логин.
    // ============================================================
    function validateLogin(text) {
        for (var i = 0; i < text.length; i++) {
            var code = text.charCodeAt(i)
            var ok =
                (code >= 0x30 && code <= 0x39) ||
                (code >= 0x41 && code <= 0x5A) ||
                (code >= 0x61 && code <= 0x7A) ||
                code === 0x5F ||
                code === 0x2D
            if (!ok) {
                return { valid: false, message: "Логин: 3–100 символов, латиница, цифры, _ и -" }
            }
        }
        if (text.length < 3 || text.length > 100) {
            return { valid: false, message: "Логин: 3–100 символов" }
        }
        return { valid: true, message: "" }
    }

    // ============================================================
    // Пароль.
    // ============================================================
    function validatePassword(text) {
        if (text.length < 12) {
            return { valid: false, message: "Пароль должен быть не короче 12 символов" }
        }
        var hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false
        for (var i = 0; i < text.length; i++) {
            var code = text.charCodeAt(i)
            if (code >= 0x41 && code <= 0x5A) hasUpper = true
            else if (code >= 0x61 && code <= 0x7A) hasLower = true
            else if (code >= 0x30 && code <= 0x39) hasDigit = true
            else if (code === 0x21 || code === 0x40 || code === 0x23 || code === 0x24 ||
                     code === 0x25 || code === 0x5E || code === 0x26 || code === 0x2A ||
                     code === 0x28 || code === 0x29 || code === 0x2D || code === 0x5F ||
                     code === 0x3D || code === 0x2B) hasSpecial = true
        }
        if (!hasUpper)   return { valid: false, message: "Пароль должен содержать заглавную букву" }
        if (!hasLower)   return { valid: false, message: "Пароль должен содержать строчную букву" }
        if (!hasDigit)   return { valid: false, message: "Пароль должен содержать цифру" }
        if (!hasSpecial) return { valid: false, message: "Пароль должен содержать спецсимвол (!@#$%^&* и др.)" }
        return { valid: true, message: "" }
    }
}
