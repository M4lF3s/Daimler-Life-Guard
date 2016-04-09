module.exports =
  warntimeHeadPose: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.headPose == 1
      warning = true
      if this.warntimeHeadPose == null
        this.warntimeHeadPose = date
      if (date - this.warntimeHeadPose) >= 5000
        critical = true

#    unconsciousnessWarning: warning
#    unconsciousnessCritical: critical
    unconsciousnessWarning: false
    unconsciousnessCritical: false

