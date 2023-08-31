#define seq(a, b) (strcmp((a), (b)) == 0)

#define SIMPLE_ALLOC(type, body) \
static void de_alloc_##type(WrenVM* vm) { \
	type* v = (type*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(type)); \
	body \
}

#define SIMPLE_FINALIZE(type, body) \
static void de_finalize_##type(void* data) { \
	type* v = (type*) data; \
	if (!v) return; \
	body \
}

#define BIND(method) static void de_bind_##method(WrenVM* vm)
#define CHECKED_FOREIGN(vm, type, var, slot) type* var = (type*)wrenGetSlotForeign(vm, slot); \
if (var == NULL) { \
	wrenSetSlotString(vm, 0, "Invalid object."); \
	wrenAbortFiber(vm, 0); \
	return; \
}
#define CHECKED_FOREIGN_INS(vm, type, var, slot) type** var##_tmp = (type**)wrenGetSlotForeign(vm, slot); \
if (*var##_tmp == NULL) { \
	wrenSetSlotString(vm, 0, "Invalid object."); \
	wrenAbortFiber(vm, 0); \
	return; \
} \
type* var = *var##_tmp;

#define NEW_FOREIGN(vm, name, type, var, slot, class_slot) wrenGetVariable(vm, "main", name, class_slot); \
type* var = (type*)wrenSetSlotNewForeign(vm, slot, class_slot, sizeof(type));

#define WREN_ASSERT(vm, x, msg) if (!x) { wrenSetSlotString(vm, 0, msg); wrenAbortFiber(vm, 0); return; }

#pragma region smol_v2_t
SIMPLE_ALLOC(smol_v2_t, { v->x = v->y = 0.0f; })
SIMPLE_FINALIZE(smol_v2_t, {})

BIND(vec2_get_x) {
	CHECKED_FOREIGN(vm, smol_v2_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->x);
}

BIND(vec2_get_y) {
	CHECKED_FOREIGN(vm, smol_v2_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->y);
}

BIND(vec2_set_x) {
	CHECKED_FOREIGN(vm, smol_v2_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->x = (float)value;
}

BIND(vec2_set_y) {
	CHECKED_FOREIGN(vm, smol_v2_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->y = (float)value;
}

BIND(vec2_set_all) {
	CHECKED_FOREIGN(vm, smol_v2_t, v, 0);
	double x = wrenGetSlotDouble(vm, 1);
	double y = wrenGetSlotDouble(vm, 2);
	v->x = (float)x;
	v->y = (float)y;
}

BIND(vec2_dot) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	wrenSetSlotDouble(vm, 0, smol_v2_dot(*a, *b));
}

BIND(vec2_wedge) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	wrenSetSlotDouble(vm, 0, smol_v2_wedge(*a, *b));
}

BIND(vec2_len) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v2_len(*a));
}

BIND(vec2_len_sq) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v2_len_sq(*a));
}

BIND(vec2_hadam) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 2);
	smol_v2_t res = smol_v2_hadam(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_norm) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 1);
	smol_v2_t res = smol_v2_norm(*a);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_mix) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	double t = wrenGetSlotDouble(vm, 2);

	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 3);
	smol_v2_t res = smol_v2_mix(*a, *b, (float)t);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_neg) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 1);
	smol_v2_t res = smol_v2_neg(*a);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_add) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 2);
	smol_v2_t res = smol_v2_add(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_sub) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v2_t, b, 1);
	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 2);
	smol_v2_t res = smol_v2_sub(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_mul) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 2);
	smol_v2_t res = smol_v2_mul((float)b, *a);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

BIND(vec2_div) {
	CHECKED_FOREIGN(vm, smol_v2_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec2", smol_v2_t, nv, 0, 2);
	smol_v2_t res = smol_v2_div(*a, (float)b);
	memcpy(nv->v, res.v, sizeof(float) * 2);
}

#pragma endregion

#pragma region smol_v3_t
SIMPLE_ALLOC(smol_v3_t, { v->x = v->y = v->z = 0.0f; })
SIMPLE_FINALIZE(smol_v3_t, {})

BIND(vec3_get_x) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->x);
}

BIND(vec3_get_y) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->y);
}

BIND(vec3_get_z) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->z);
}

BIND(vec3_set_x) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->x = (float)value;
}

BIND(vec3_set_y) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->y = (float)value;
}

BIND(vec3_set_z) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->z = (float)value;
}

BIND(vec3_set_all) {
	CHECKED_FOREIGN(vm, smol_v3_t, v, 0);
	double x = wrenGetSlotDouble(vm, 1);
	double y = wrenGetSlotDouble(vm, 2);
	double z = wrenGetSlotDouble(vm, 3);
	v->x = (float)x;
	v->y = (float)y;
	v->z = (float)z;
}

BIND(vec3_dot) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	wrenSetSlotDouble(vm, 0, smol_v3_dot(*a, *b));
}

BIND(vec3_cross) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_cross(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_len) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v3_len(*a));
}

BIND(vec3_len_sq) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v3_len_sq(*a));
}

BIND(vec3_hadam) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_hadam(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_norm) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 1);
	smol_v3_t res = smol_v3_norm(*a);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_mix) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	double t = wrenGetSlotDouble(vm, 2);

	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 3);
	smol_v3_t res = smol_v3_mix(*a, *b, (float)t);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_neg) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 1);
	smol_v3_t res = smol_v3_neg(*a);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_add) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_add(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_sub) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, b, 1);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_sub(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_mul) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_mul((float)b, *a);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(vec3_div) {
	CHECKED_FOREIGN(vm, smol_v3_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_v3_div(*a, (float)b);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}
#pragma endregion

#pragma region smol_v4_t
SIMPLE_ALLOC(smol_v4_t, { v->x = v->y = v->z = v->w = 0.0f; })
SIMPLE_FINALIZE(smol_v4_t, {})

BIND(vec4_get_x) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->x);
}

BIND(vec4_get_y) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->y);
}

BIND(vec4_get_z) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->z);
}

BIND(vec4_get_w) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	wrenSetSlotDouble(vm, 0, (double)v->w);
}

BIND(vec4_set_x) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->x = (float)value;
}

BIND(vec4_set_y) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->y = (float)value;
}

BIND(vec4_set_z) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->z = (float)value;
}

BIND(vec4_set_w) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	double value = wrenGetSlotDouble(vm, 1);
	v->w = (float)value;
}

BIND(vec4_set_all) {
	CHECKED_FOREIGN(vm, smol_v4_t, v, 0);
	double x = wrenGetSlotDouble(vm, 1);
	double y = wrenGetSlotDouble(vm, 2);
	double z = wrenGetSlotDouble(vm, 3);
	double w = wrenGetSlotDouble(vm, 4);
	v->x = (float)x;
	v->y = (float)y;
	v->z = (float)z;
	v->w = (float)w;
}

BIND(vec4_dot) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v4_t, b, 1);
	wrenSetSlotDouble(vm, 0, smol_v4_dot(*a, *b));
}

BIND(vec4_len) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v4_len(*a));
}

BIND(vec4_len_sq) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	wrenSetSlotDouble(vm, 0, smol_v4_len_sq(*a));
}

BIND(vec4_hadam) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v4_t, b, 1);
	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 2);
	smol_v4_t res = smol_v4_hadam(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_norm) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 1);
	smol_v4_t res = smol_v4_norm(*a);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_mix) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v4_t, b, 1);
	double t = wrenGetSlotDouble(vm, 2);

	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 3);
	smol_v4_t res = smol_v4_mix(*a, *b, (float)t);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_neg) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 1);
	smol_v4_t res = smol_v4_neg(*a);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_add) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v4_t, b, 1);
	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 2);
	smol_v4_t res = smol_v4_add(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_sub) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v4_t, b, 1);
	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 2);
	smol_v4_t res = smol_v4_sub(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_mul) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 2);
	smol_v4_t res = smol_v4_mul((float)b, *a);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(vec4_div) {
	CHECKED_FOREIGN(vm, smol_v4_t, a, 0);
	double b = wrenGetSlotDouble(vm, 1);

	NEW_FOREIGN(vm, "Vec4", smol_v4_t, nv, 0, 2);
	smol_v4_t res = smol_v4_div(*a, (float)b);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}
#pragma endregion

#pragma region smol_quat_t
SIMPLE_ALLOC(smol_quat_t, {
	smol_quat_t q = smol_quat_identity();
	v->x = q.x;
	v->y = q.y;
	v->z = q.z;
	v->w = q.w;
})
SIMPLE_FINALIZE(smol_quat_t, {})

BIND(quat_conjugated) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	NEW_FOREIGN(vm, "Quat", smol_quat_t, nv, 0, 1);
	smol_quat_t res = smol_quat_conjugate(*a);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(quat_axis_angle) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, axis, 1);
	double angle = wrenGetSlotDouble(vm, 2);

	smol_quat_t res = smol_quat_from_axis_angle(*axis, angle);
	memcpy(a->v, res.v, sizeof(float) * 4);
}

BIND(quat_rotate_vec3) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, vec, 1);

	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 2);
	smol_v3_t res = smol_quat_rotate_v3(*a, *vec);
	memcpy(nv->v, res.v, sizeof(float) * 3);
}

BIND(quat_lerp) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	CHECKED_FOREIGN(vm, smol_quat_t, b, 1);
	double t = wrenGetSlotDouble(vm, 2);

	NEW_FOREIGN(vm, "Quat", smol_quat_t, nv, 0, 3);
	smol_quat_t res = smol_quat_lerp(*a, *b, (float)t);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(quat_slerp) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	CHECKED_FOREIGN(vm, smol_quat_t, b, 1);
	double t = wrenGetSlotDouble(vm, 2);

	NEW_FOREIGN(vm, "Quat", smol_quat_t, nv, 0, 3);
	smol_quat_t res = smol_quat_slerp(*a, *b, (float)t);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

BIND(quat_mul) {
	CHECKED_FOREIGN(vm, smol_quat_t, a, 0);
	CHECKED_FOREIGN(vm, smol_quat_t, b, 1);

	NEW_FOREIGN(vm, "Quat", smol_quat_t, nv, 0, 2);
	smol_v4_t res = smol_quat_mul(*a, *b);
	memcpy(nv->v, res.v, sizeof(float) * 4);
}

#pragma endregion

#pragma region de_object
SIMPLE_ALLOC(de_object_t, { de_object_init(v); })
SIMPLE_FINALIZE(de_object_t, { })

BIND(obj_get_prop) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	const char* name = wrenGetSlotString(vm, 1);
	de_prop_t* prop = de_object_get_prop(obj, name);

	if (!prop) {
		wrenSetSlotString(vm, 0, "Invalid property name.");
		wrenAbortFiber(vm, 0);
		return;
	}

	switch (prop->type) {
		case DE_PROP_STRING: wrenSetSlotString(vm, 0, prop->str_prop); break;
		case DE_PROP_FLOAT: wrenSetSlotDouble(vm, 0, (double)prop->flt_prop); break;
		case DE_PROP_INT: wrenSetSlotDouble(vm, 0, (double)prop->int_prop); break;
		case DE_PROP_BOOL: wrenSetSlotBool(vm, 0, prop->int_prop > 0 ? 1 : 0); break;
	}
}

BIND(obj_set_prop) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	const char* name = wrenGetSlotString(vm, 1);
	de_prop_t* prop = de_object_get_prop(obj, name);
	if (!prop) {
		wrenSetSlotString(vm, 0, "Invalid property name.");
		wrenAbortFiber(vm, 0);
		return;
	}
	
	WrenType type = wrenGetSlotType(vm, 2);
	switch (type) {
		default: {
			WREN_ASSERT(vm, 0, "Invalid property type.");
		} break;
		case WREN_TYPE_BOOL: {
			WREN_ASSERT(vm, prop->type == DE_PROP_BOOL, "Expected a BOOL prop.");
			prop->int_prop = wrenGetSlotBool(vm, 2);
		} break;
		case WREN_TYPE_NUM: {
			WREN_ASSERT(vm, prop->type == DE_PROP_INT || prop->type == DE_PROP_FLOAT, "Expected an INT or FLOAT prop.");
			double val = wrenGetSlotDouble(vm, 2);
			if (val == (int)val) {
				prop->int_prop = (int)val;
			}
			else {
				prop->flt_prop = (float)val;
			}
		} break;
		case WREN_TYPE_STRING: {
			WREN_ASSERT(vm, prop->type == DE_PROP_STRING, "Expected a STRING prop.");
			const char* str = wrenGetSlotString(vm, 2);
			strcpy(prop->str_prop, str);
		} break;
	}
}

BIND(obj_get_name) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	wrenSetSlotString(vm, 0, obj->name);
}

BIND(obj_get_tag) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	wrenSetSlotString(vm, 0, obj->tag);
}

BIND(obj_get_life) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	wrenSetSlotDouble(vm, 0, (double)obj->life);
}

BIND(obj_set_life) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	double life = wrenGetSlotDouble(vm, 1);
	obj->life = life;
}

BIND(obj_get_position) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 1);
	memcpy(nv->v, obj->position.v, sizeof(float) * 3);
}

BIND(obj_set_position) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, nv, 1);
	memcpy(obj->position.v, nv->v, sizeof(float) * 3);
}

BIND(obj_get_rotation) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	NEW_FOREIGN(vm, "Quat", smol_quat_t, nv, 0, 1);
	memcpy(nv->v, obj->rotation.v, sizeof(float) * 4);
}

BIND(obj_set_rotation) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	CHECKED_FOREIGN(vm, smol_quat_t, nv, 1);
	memcpy(obj->rotation.v, nv->v, sizeof(float) * 4);
}

BIND(obj_get_scale) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	NEW_FOREIGN(vm, "Vec3", smol_v3_t, nv, 0, 1);
	memcpy(nv->v, obj->scale.v, sizeof(float) * 3);
}

BIND(obj_set_scale) {
	CHECKED_FOREIGN_INS(vm, de_object_t, obj, 0);
	CHECKED_FOREIGN(vm, smol_v3_t, nv, 1);
	memcpy(obj->scale.v, nv->v, sizeof(float) * 3);
}

#pragma endregion

typedef struct _de_class_reg_t {
	char* name;
	WrenForeignClassMethods methods;
} de_class_reg_t;

#define CL_REG_ENTRY(name, type) { name, { de_alloc_##type, de_finalize_##type } }
#define CL_REG_ENTRY_GUARD { NULL, { NULL, NULL } }

typedef struct _de_method_reg_t {
	char* className;
	char* signature;
	WrenForeignMethodFn impl;
} de_method_reg_t;

#define MT_REG_ENTRY(cls, sig, impl) { cls, sig, impl }
#define MT_REG_ENTRY_GUARD { NULL, NULL, NULL }

static const de_class_reg_t gamelib_classes[] = {
	CL_REG_ENTRY("Vec2", smol_v2_t),
	CL_REG_ENTRY("Vec3", smol_v3_t),
	CL_REG_ENTRY("Vec4", smol_v4_t),
	CL_REG_ENTRY("Quat", smol_quat_t),
	CL_REG_ENTRY("de_object", de_object_t),
	CL_REG_ENTRY_GUARD
};

static const de_method_reg_t gamelib_methods[] = {
	MT_REG_ENTRY("de_object", "[_]", de_bind_obj_get_prop),
	MT_REG_ENTRY("de_object", "[_]=(_)", de_bind_obj_set_prop),
	MT_REG_ENTRY("de_object", "position", de_bind_obj_get_position),
	MT_REG_ENTRY("de_object", "position=(_)", de_bind_obj_set_position),
	MT_REG_ENTRY("de_object", "rotation", de_bind_obj_get_rotation),
	MT_REG_ENTRY("de_object", "rotation=(_)", de_bind_obj_set_rotation),
	MT_REG_ENTRY("de_object", "scale", de_bind_obj_get_scale),
	MT_REG_ENTRY("de_object", "scale=(_)", de_bind_obj_set_scale),
	MT_REG_ENTRY("de_object", "tag", de_bind_obj_get_tag),
	MT_REG_ENTRY("de_object", "name", de_bind_obj_get_name),
	MT_REG_ENTRY("de_object", "life", de_bind_obj_get_life),
	MT_REG_ENTRY("de_object", "life=(_)", de_bind_obj_set_life),

	MT_REG_ENTRY("Vec2", "x", de_bind_vec2_get_x),
	MT_REG_ENTRY("Vec2", "y", de_bind_vec2_get_y),
	MT_REG_ENTRY("Vec2", "x=(_)", de_bind_vec2_set_x),
	MT_REG_ENTRY("Vec2", "y=(_)", de_bind_vec2_set_y),
	MT_REG_ENTRY("Vec2", "set(_,_)", de_bind_vec2_set_all),
	MT_REG_ENTRY("Vec2", "dot(_)", de_bind_vec2_dot),
	MT_REG_ENTRY("Vec2", "wedge(_)", de_bind_vec2_wedge),
	MT_REG_ENTRY("Vec2", "normalized()", de_bind_vec2_norm),
	MT_REG_ENTRY("Vec2", "mix(_,_)", de_bind_vec2_mix),
	MT_REG_ENTRY("Vec2", "hadam(_)", de_bind_vec2_hadam),
	MT_REG_ENTRY("Vec2", "-", de_bind_vec2_neg),
	MT_REG_ENTRY("Vec2", "+(_)", de_bind_vec2_add),
	MT_REG_ENTRY("Vec2", "-(_)", de_bind_vec2_sub),
	MT_REG_ENTRY("Vec2", "*(_)", de_bind_vec2_mul),
	MT_REG_ENTRY("Vec2", "/(_)", de_bind_vec2_div),
	MT_REG_ENTRY("Vec2", "len", de_bind_vec2_len),
	MT_REG_ENTRY("Vec2", "len_sq", de_bind_vec2_len_sq),

	MT_REG_ENTRY("Vec3", "x", de_bind_vec3_get_x),
	MT_REG_ENTRY("Vec3", "y", de_bind_vec3_get_y),
	MT_REG_ENTRY("Vec3", "z", de_bind_vec3_get_z),
	MT_REG_ENTRY("Vec3", "x=(_)", de_bind_vec3_set_x),
	MT_REG_ENTRY("Vec3", "y=(_)", de_bind_vec3_set_y),
	MT_REG_ENTRY("Vec3", "z=(_)", de_bind_vec3_set_z),
	MT_REG_ENTRY("Vec3", "set(_,_,_)", de_bind_vec3_set_all),
	MT_REG_ENTRY("Vec3", "dot(_)", de_bind_vec3_dot),
	MT_REG_ENTRY("Vec3", "cross(_)", de_bind_vec3_cross),
	MT_REG_ENTRY("Vec3", "normalized()", de_bind_vec3_norm),
	MT_REG_ENTRY("Vec3", "mix(_,_)", de_bind_vec3_mix),
	MT_REG_ENTRY("Vec3", "hadam(_)", de_bind_vec3_hadam),
	MT_REG_ENTRY("Vec3", "-", de_bind_vec3_neg),
	MT_REG_ENTRY("Vec3", "+(_)", de_bind_vec3_add),
	MT_REG_ENTRY("Vec3", "-(_)", de_bind_vec3_sub),
	MT_REG_ENTRY("Vec3", "*(_)", de_bind_vec3_mul),
	MT_REG_ENTRY("Vec3", "/(_)", de_bind_vec3_div),
	MT_REG_ENTRY("Vec3", "len", de_bind_vec3_len),
	MT_REG_ENTRY("Vec3", "len_sq", de_bind_vec3_len_sq),

	MT_REG_ENTRY("Vec4", "x", de_bind_vec4_get_x),
	MT_REG_ENTRY("Vec4", "y", de_bind_vec4_get_y),
	MT_REG_ENTRY("Vec4", "z", de_bind_vec4_get_z),
	MT_REG_ENTRY("Vec4", "w", de_bind_vec4_get_w),
	MT_REG_ENTRY("Vec4", "x=(_)", de_bind_vec4_set_x),
	MT_REG_ENTRY("Vec4", "y=(_)", de_bind_vec4_set_y),
	MT_REG_ENTRY("Vec4", "z=(_)", de_bind_vec4_set_z),
	MT_REG_ENTRY("Vec4", "w=(_)", de_bind_vec4_set_w),
	MT_REG_ENTRY("Vec4", "set(_,_,_,_)", de_bind_vec4_set_all),
	MT_REG_ENTRY("Vec4", "dot(_)", de_bind_vec4_dot),
	MT_REG_ENTRY("Vec4", "normalized()", de_bind_vec4_norm),
	MT_REG_ENTRY("Vec4", "mix(_,_)", de_bind_vec4_mix),
	MT_REG_ENTRY("Vec4", "hadam(_)", de_bind_vec4_hadam),
	MT_REG_ENTRY("Vec4", "-", de_bind_vec4_neg),
	MT_REG_ENTRY("Vec4", "+(_)", de_bind_vec4_add),
	MT_REG_ENTRY("Vec4", "-(_)", de_bind_vec4_sub),
	MT_REG_ENTRY("Vec4", "*(_)", de_bind_vec4_mul),
	MT_REG_ENTRY("Vec4", "/(_)", de_bind_vec4_div),
	MT_REG_ENTRY("Vec4", "len", de_bind_vec4_len),
	MT_REG_ENTRY("Vec4", "len_sq", de_bind_vec4_len_sq),

	MT_REG_ENTRY("Quat", "x", de_bind_vec4_get_x),
	MT_REG_ENTRY("Quat", "y", de_bind_vec4_get_y),
	MT_REG_ENTRY("Quat", "z", de_bind_vec4_get_z),
	MT_REG_ENTRY("Quat", "w", de_bind_vec4_get_w),
	MT_REG_ENTRY("Quat", "x=(_)", de_bind_vec4_set_x),
	MT_REG_ENTRY("Quat", "y=(_)", de_bind_vec4_set_y),
	MT_REG_ENTRY("Quat", "z=(_)", de_bind_vec4_set_z),
	MT_REG_ENTRY("Quat", "w=(_)", de_bind_vec4_set_w),
	MT_REG_ENTRY("Quat", "set(_,_,_,_)", de_bind_vec4_set_all),
	MT_REG_ENTRY("Quat", "dot(_)", de_bind_vec4_dot),
	MT_REG_ENTRY("Quat", "normalized()", de_bind_vec4_norm),
	MT_REG_ENTRY("Quat", "mix(_,_)", de_bind_vec4_mix),
	MT_REG_ENTRY("Quat", "hadam(_)", de_bind_vec4_hadam),
	MT_REG_ENTRY("Quat", "-", de_bind_vec4_neg),
	MT_REG_ENTRY("Quat", "+(_)", de_bind_vec4_add),
	MT_REG_ENTRY("Quat", "-(_)", de_bind_vec4_sub),
	MT_REG_ENTRY("Quat", "*(_)", de_bind_quat_mul),
	MT_REG_ENTRY("Quat", "/(_)", de_bind_vec4_div),
	MT_REG_ENTRY("Quat", "len", de_bind_vec4_len),
	MT_REG_ENTRY("Quat", "len_sq", de_bind_vec4_len_sq),
	MT_REG_ENTRY("Quat", "conjugated()", de_bind_quat_conjugated),
	MT_REG_ENTRY("Quat", "lerp(_,_)", de_bind_quat_lerp),
	MT_REG_ENTRY("Quat", "slerp(_,_)", de_bind_quat_slerp),
	MT_REG_ENTRY("Quat", "set_axis_angle(_,_)", de_bind_quat_axis_angle),
	MT_REG_ENTRY("Quat", "rotate_vec3(_)", de_bind_quat_rotate_vec3),

	MT_REG_ENTRY_GUARD
};

WrenForeignMethodFn bindForeignMethod(
	WrenVM* vm,
	const char* module,
	const char* className,
	bool isStatic,
	const char* signature
) {
	for (de_method_reg_t* ptr = gamelib_methods; ptr->className; ++ptr) {
		if (seq(className, ptr->className) && seq(signature, ptr->signature)) {
			return ptr->impl;
		}
	}

	return NULL;
}

WrenForeignClassMethods bindForeignClass(
	WrenVM* vm,
	const char* module,
	const char* className
) {
	for (de_class_reg_t* ptr = gamelib_classes; ptr->name; ++ptr) {
		if (seq(className, ptr->name)) {
			return ptr->methods;
		}
	}

	WrenForeignClassMethods methods = { 0 };
	return methods;
}
