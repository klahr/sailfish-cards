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

// Player colours, the same in every game: you are blue, then red, green
// and amber for the others.
var playerColors = ["#4a9dff", "#ff5252", "#5ad16a", "#ffb340"]
function playerColor(player) { return playerColors[player >= 1 && player <= 3 ? player : 0] }
