extend = require('util')._extend

module.exports =
  maxData: 1000
  targetFps: 10,
  maxRecordJoinTime: 100,
  sensorData: [],
  healthCondition: {},

  post: (req, res) ->
    data = req.body
    timestamp = Date.now()
    if this.sensorData.length == 0
      # No data yet at all -> new record
      data.timestamp = timestamp
      this.sensorData.push data
    else
      latest = this.latestData()
      if timestamp - latest.timestamp > this.maxRecordJoinTime
        # Create a new record
        data.timestamp = timestamp
        this.sensorData = [req.body].concat this.sensorData
        if this.sensorData.length > this.maxData
          this.sensorData.pop()
      else
        # Append to existing record
        latest = extend(latest, data)

    # Successfully received new data -> Analyze
    this.healthCondition = AnalysisService.analyze this.latestData()
    console.log "Data count: #{this.sensorData.length}"

    # Finish request
    res.ok()

  latestData: () ->
    if this.sensorData.length > 0
      this.sensorData[0]
    else
      {}

  latest: (req, res) ->
    data = this.sensorData[0]
    res.json(data)
