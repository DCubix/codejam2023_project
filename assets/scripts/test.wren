class Test {
    construct new() {}

    static on_create(object) {
        System.print("Hello from on_create")
    }

    static on_update(object, dt) {
        var yrot = Quat.new()
        var ax = Vec3.new()
        ax.set(0, 0, 1)
        yrot.set_axis_angle(ax, dt)

        object.rotation = object.rotation * yrot
    }

    static on_destroy(object) {

    }
}
