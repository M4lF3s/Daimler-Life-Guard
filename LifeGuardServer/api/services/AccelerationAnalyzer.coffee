module.exports =
  analyze: (isShaking) ->
    if isShaking
      1
    else
      0
