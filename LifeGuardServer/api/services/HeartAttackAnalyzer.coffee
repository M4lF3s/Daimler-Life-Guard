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
      if (date - this.warntimePulse) >= 3000
        critical = true
    else
      this.warntimePulse = null
      this.warntimeHeadPose = null

#    if sensorAnalyzers.pose == 1
#      warning = true
#      if this.warntimeHeadPose == null
#        this.warntimeHeadPose = date
#      if (date - this.warntimeHeadPose) >= 3000
#        critical = true

    heart:
      warning: warning
      critical: critical
