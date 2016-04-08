module.exports =
  warntimeEyesOpen: null
  warntimeHeadPose: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.eyesOpen == 1
      warning = true
      if this.warntimeEyesOpen == null
        this.warntimeEyesOpen = date
      if (date - this.warntimeEyesOpen) >= 5000
        critical = true

    if sensorAnalyzers.headPose == 1
      warning = true
      if this.warntimeHeadPose == null
        this.warntimeHeadPose = date
      if (date - this.warntimeHeadPose) >= 5000
        critical = true

    unconsciousnessWarning: warning
    unconsciousnessCritical: critical

