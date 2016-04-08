module.exports =
  analyze: (pulse) ->
    if pulse is undefined
      0 # Driver does not touch steering wheel with both hands -> Not our problem
    else if pulse < 40
      1 - (Math.max(pulse - 20, 0) / 20)
    else if pulse >= 180
      Math.min((pulse - 180) / 10, 1)
    else
      0
