module.exports =
  analyze: (pulse) ->
    if pulse < 40
      1 - (Math.max(pulse - 20, 0) / 20)
    else if pulse >= 180
      Math.min((pulse - 180) / 10, 1)
    else
      0

