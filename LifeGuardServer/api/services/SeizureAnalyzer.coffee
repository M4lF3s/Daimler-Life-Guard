module.exports =
  warntimeMuscle: null
  warntimeAcceleration: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.isSeizure == 1
      warning = true
      if this.warntimeMuscle == null
        this.warntimeMuscle = date
      if (date - this.warntimeMuscle) >= 2500
        critical = true
    else
      this.warntimeMuscle = null

    if sensorAnalyzers.isShaking == 1
      warning = true
      if this.warntimeAcceleration == null
        this.warntimeAcceleration = date
      if (date - this.warntimeAcceleration) >= 2500
        critical = true
    else
      this.warntimeAcceleration = null

    seizure:
      warning: warning
      critical: critical

