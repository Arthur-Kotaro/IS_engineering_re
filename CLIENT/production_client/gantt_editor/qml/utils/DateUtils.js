.pragma library

function parseDdMmYyyy(text)
{
    if (!text) return null
    var parts = text.split(".")
    if (parts.length !== 3) return null
    var d = parseInt(parts[0], 10)
    var m = parseInt(parts[1], 10) - 1
    var y = parseInt(parts[2], 10)
    if (isNaN(d) || isNaN(m) || isNaN(y)) return null
    var date = new Date(y, m, d)
    if (isNaN(date.getTime())) return null
    // Проверка на «переполнение» (31.02 → 03.03): сравниваем компоненты
    if (date.getFullYear() !== y || date.getMonth() !== m || date.getDate() !== d) return null
    return date
}

function formatDdMmYyyy(date)
{
    if (!isValidDate(date)) return ""
    var d = date.getDate()
    var m = date.getMonth() + 1
    var y = date.getFullYear()
    return (d < 10 ? "0" : "") + d + "." + (m < 10 ? "0" : "") + m + "." + y
}

function isValidDate(date)
{
    return date !== undefined && date !== null && !isNaN(new Date(date).getTime())
}

function addDays(date, n)
{
    if (!isValidDate(date)) return null
    var d = new Date(date)
    d.setDate(d.getDate() + n)
    d.setHours(0, 0, 0, 0)
    return d
}

function startOfDay(date)
{
    if (!isValidDate(date)) return null
    var d = new Date(date)
    d.setHours(0, 0, 0, 0)
    return d
}

function daysBetween(a, b)
{
    if (!isValidDate(a) || !isValidDate(b)) return 0
    return Math.floor((startOfDay(b) - startOfDay(a)) / 86400000)
}
