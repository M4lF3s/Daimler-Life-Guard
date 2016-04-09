module.exports =
  analyze: (pose) ->
    if pose == undefined
      1
    else if Math.abs(pose.rotation.x) > 50
      1
    else if Math.abs(pose.rotation.y) > 50
      1
    else if Math.abs(pose.rotation.z) > 50
      1
    else
      0
