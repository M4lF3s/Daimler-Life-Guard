module.exports =
  analyze: (eyesOpen) ->
    if eyesOpen < 0.2
      1
    else
      0
