module.exports =
  warntimePulse: null
  warntimeHeadPose: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.pulse == 1
      warning = true
      if this.warntimePulse == null
        this.warntimePulse = date
      if (date - this.warntimePulse) >= 5000
        critical = true

    if sensorAnalyzers.headPose == 1
      warning = true
      if this.warntimeHeadPose == null
        this.warntimeHeadPose = date
      if (date - this.warntimeHeadPose) >= 5000
        critical = true

    heartWarning: warning
    heartCritical: critical
