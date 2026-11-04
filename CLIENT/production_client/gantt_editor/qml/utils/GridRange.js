.pragma library

function isValidDate(d)
{
    return d !== undefined && d !== null && !isNaN(new Date(d).getTime())
}

function daysFromStart(dateValue, displayStart)
{
    return Math.floor((new Date(dateValue) - displayStart) / 86400000)
}

function dateToX(dateValue, displayStart, dayWidth)
{
    if (!isValidDate(dateValue)) return -1
    return daysFromStart(dateValue, displayStart) * dayWidth
}

function computeLocalDisplayRange(linkedStart, linkedEnd, earliest, latest)
{
    var s = new Date(linkedStart)
    var e = new Date(linkedEnd)

    var durationDays = Math.floor((e - s) / 86400000) + 1
    if (durationDays < 30)
    {
        var ms = new Date(s.getFullYear(), s.getMonth(), 1)
        ms.setDate(ms.getDate() - 14)
        ms.setHours(0, 0, 0, 0)
        var me = new Date(e.getFullYear(), e.getMonth() + 1, 0)
        me.setDate(me.getDate() + 14)
        me.setHours(23, 59, 59, 999)
        s = ms
        e = me
    }
    else
    {
        s.setDate(s.getDate() - 14)
        s.setHours(0, 0, 0, 0)
        e.setDate(e.getDate() + 14)
        e.setHours(23, 59, 59, 999)
    }

    if (isValidDate(earliest) && earliest < s) s = new Date(earliest)
    if (isValidDate(latest) && latest > e) e = new Date(latest)

    while (s.getDay() !== 1) s.setDate(s.getDate() - 1)

    return { start: s, end: e }
}
