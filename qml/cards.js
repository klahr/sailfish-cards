.pragma library

var ranks = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]
var suits = ["♣", "♦", "♥", "♠"]

function rank(id) { return id % 13 + 1 }
function suit(id) { return Math.floor(id / 13) }
// Rank when aces are high.
function high(id) { var r = rank(id); return r === 1 ? 14 : r }
function rankText(id) { return ranks[id % 13] }
function suitText(id) { return suits[suit(id)] }
function isRed(id) { var s = suit(id); return s === 1 || s === 2 }
function isFace(id) { return id >= 0 && rank(id) > 10 }
function name(id) { return rankText(id) + suitText(id) }

function names(ids) {
    var out = []
    for (var i = 0; i < ids.length; ++i)
        out.push(name(ids[i]))
    return out.join(" ")
}

// Player colours, the same in every game: you are blue, then red, green,
// amber and violet for the others.
var playerColors = ["#4a9dff", "#ff5252", "#5ad16a", "#ffb340", "#c58bff"]
function playerColor(player) { return playerColors[player >= 1 && player < playerColors.length ? player : 0] }

// A player's colour gone grey and faint, for one who has lost the connection
// and has no bot playing for them.
function awayColor(player) {
    var c = playerColor(player)
    var grey = Math.round(0.299 * parseInt(c.substr(1, 2), 16) + 0.587 * parseInt(c.substr(3, 2), 16)
                          + 0.114 * parseInt(c.substr(5, 2), 16))
    var hex = (grey < 16 ? "0" : "") + grey.toString(16)
    return "#80" + hex + hex + hex
}
