
module.exports =
  maxData: 1000
  sensorData: []

  post: (req, res) ->
    data = req.body
    timestamp = Date.now()
    if this.sensorData.length == 0
      this.sensorData.push(data)

    this.sensorData = [req.body].concat this.sensorData
    if this.sensorData.length > this.maxData
      this.sensorData.pop()

    console.log this.sensorData.length
    res.ok()


  latestData: () ->
    sensorData[0]


