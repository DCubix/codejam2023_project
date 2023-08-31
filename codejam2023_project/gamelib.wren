foreign class Vec2 {
    construct new() {}

    foreign x
    foreign y
    foreign x = (value)
    foreign y = (value)

    foreign len
    foreign len_sq

    foreign set(x,y)

    foreign dot(b)
    foreign wedge(b)
    foreign normalized()
    foreign mix(b,t)
    foreign hadam(b)

    foreign -
    foreign +(b)
    foreign -(b)
    foreign *(b)
    foreign /(b)
}

foreign class Vec3 {
    construct new() {}

    foreign x
    foreign y
    foreign z
    foreign x = (value)
    foreign y = (value)
    foreign z = (value)

    foreign len
    foreign len_sq

    foreign set(x,y,z)

    foreign dot(b)
    foreign cross(b)
    foreign normalized()
    foreign mix(b,t)
    foreign hadam(b)

    foreign -
    foreign +(b)
    foreign -(b)
    foreign *(b)
    foreign /(b)
}

foreign class Vec4 {
    construct new() {}

    foreign x
    foreign y
    foreign z
    foreign w
    foreign x = (value)
    foreign y = (value)
    foreign z = (value)
    foreign w = (value)

    foreign len
    foreign len_sq

    foreign set(x,y,z,w)

    foreign dot(b)
    foreign normalized()
    foreign mix(b,t)
    foreign hadam(b)

    foreign -
    foreign +(b)
    foreign -(b)
    foreign *(b)
    foreign /(b)
}

foreign class Quat {
    construct new() {}

    foreign x
    foreign y
    foreign z
    foreign w
    foreign x = (value)
    foreign y = (value)
    foreign z = (value)
    foreign w = (value)

    foreign len
    foreign len_sq

    foreign set(x,y,z,w)

    foreign dot(b)
    foreign normalized()
    foreign conjugated()
    foreign mix(b,t)
    foreign hadam(b)

    foreign lerp(b,t)
    foreign slerp(b,t)
    foreign set_axis_angle(axis,angle)
    foreign rotate_vec3(v3)

    foreign -
    foreign +(b)
    foreign -(b)
    foreign *(b)
    foreign /(b)
}

foreign class de_object {
    foreign name
    foreign tag
    
    foreign life
    foreign life = (value)

    foreign position
    foreign position = (value)
    foreign rotation
    foreign rotation = (value)
    foreign scale
    foreign scale = (value)

    // property getter/setter
    foreign [key]
    foreign [key]=(value)
}
