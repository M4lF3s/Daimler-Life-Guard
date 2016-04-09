module.exports =
  warntimeHeadPose: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.pose == 1
      warning = true
      if this.warntimeHeadPose == null
        this.warntimeHeadPose = date
      if (date - this.warntimeHeadPose) >= 3000
        critical = true
    else
      this.warntimeHeadPose = null

    unconsciousness:
      warning: warning
      critical: critical
#    unconsciousnessWarning: false
#    unconsciousnessCritical: false

