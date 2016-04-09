module.exports =
  warntimeEyesOpen: null

  analyze: (sensorAnalyzers) ->
    warning = false
    critical = false
    date = Date.now()

    if sensorAnalyzers.eyesOpen == 1
      warning = true
      if this.warntimeEyesOpen == null
        this.warntimeEyesOpen = date
      if (date - this.warntimeEyesOpen) >= 3000
        critical = true
    else
      this.warntimeEyesOpen = null

    sleeping:
      warning: warning
      critical: critical
