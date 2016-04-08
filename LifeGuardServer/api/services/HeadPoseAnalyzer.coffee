module.exports =
  analyze: (pose) ->
    if pose == undefined
      1
    else if pose.rotation.x > 50
      1
    else if pose.rotation.y > 50
      1
    else if pose.rotation.z > 50
      1
    else 
      0
