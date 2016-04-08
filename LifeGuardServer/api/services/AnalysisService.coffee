extend = require('util')._extend

module.exports =
  # Analyzes raw sensor values and rates them for severity.
  # Input: Current Sensor Value
  # Output: Number, 0-1, 0 = harmless, 1 = critical
  sensorAnalyzers:
    pulse: (pulse) -> PulseAnalyzer.analyze(pulse)
    eyesOpen: () ->
    headPose: () ->
    muscleActivity: () ->

  # Analyzes the preprocessed sensor values with respect to a specific disease
  # Input: Map containing all preprocessed sensor values. Format: "name" -> severity [0-1]
  # Output: Map containing a warning and a severity flag for the symptom. Format:
  # {
  #   "<symptom>Warning": [true|false]
  #   "<symptom>Critical": [true|false]
  # }
  symptomAnalyzers:
    heartAttack: () ->
    unconsciousness: () ->
    sleeping: () ->
    seizure: () ->

  analyze: (latestMeasurement) ->
    analyzedSensors = {}
    result = {}

    # Step 1: Analyze the latest value of each sensor
    for property, analyzer of this.sensorAnalyzers
      analyzedSensors[property] = analyzer(latestMeasurement[property])
    console.log latestMeasurement
    console.log(analyzedSensors)

    # Step 2: Analyze the preprocessed sensor values for symptoms
    for property, analyzer of this.symptomAnalyzers
      result = extend(result, analyzer(analyzedSensors));

    result
