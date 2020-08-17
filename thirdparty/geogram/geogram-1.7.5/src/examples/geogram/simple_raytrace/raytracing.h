/*
 *  Copyright (c) 2012-2014, Bruno Levy All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef RAYTRACING_H
#define RAYTRACING_H



#include <geogram/basic/geometry.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>

#ifdef RAYTRACE_GUI
#include <geogram_gfx/third_party/ImGui/imgui.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#endif

// We have everything in the .h (a bit ugly but it is used
// by two demos, so it is simpler like that), this makes
// Clang complain about vtbls generation (make it ignore
// the warning).
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

/**
 * \file raytracing.h
 * \brief Implementation of a simple raytracer, to demonstrate
 *   AABB usage. See MeshObject below.
 */

namespace GEO {

    /*******************************************************************/
    
    class Object;

    /**
     * \brief The small constant to shift a little bit ray intersections 
     *  in order to avoid false positives when detecting shadows.
     */
    const double epsilon_t = 1e-6;

    /**
     * \brief Multiplies two vec3 componentwise.
     * \param[in] U , V the two vec3 to be multiplied
     * \return the component-wise product
     */
    inline vec3 mul(const vec3& U, const vec3& V) {
	return vec3(
	    U.x*V.x,
	    U.y*V.y,
	    U.z*V.z	    
	);
    }

    /*******************************************************************/

    /**
     * \brief The Camere.
     * \details Launches rays and stores the resulting image.
     */
    class Camera {
    public:

	/**
	 * \brief Camera constructor.
	 * \param[in] position the position of the camera.
	 * \param[in] target a point that is looked at.
	 * \param[in] image_width , image_height dimension of the image.
	 * \param[in] zoom viewing angle of the camera, in degrees.
	 */
	Camera(
	    const vec3& position,
	    const vec3& target,
	    index_t image_width,
	    index_t image_height,
	    double zoom = 40.0
	) :
	    image_width_(image_width),
	    image_height_(image_height),
	    image_(image_width*image_height*3),
	    bpp_(3)
	{
	    update(position, target, zoom);
	}

	/**
	 * \brief Camera constructor.
	 * \details Viewing parameters are not initialized.
	 * \param[in] image_width , image_height dimension of the image.
	 * \param[in] bpp bytes per pixel, 3 or 4
	 */	
	Camera(
	    index_t image_width, index_t image_height, index_t bpp
	) : image_width_(image_width),
	    image_height_(image_height),
	    image_(image_width*image_height*bpp),
	    bpp_(bpp)
	{
	    geo_assert(bpp == 3 || bpp == 4);
	}

	/**
	 * \brief Resizes the image.
	 * \param[in] new_width , new_height new image size, in pixels.
	 */
	void resize(index_t new_width, index_t new_height) {
	    image_width_ = new_width;
	    image_height_ = new_height;
	    image_.resize(image_width_*image_height_*bpp_);
	}
	
	/**
	 * \brief Updates the camera parameters.
	 * \param[in] position the position of the camera.
	 * \param[in] target a point that is looked at.
	 * \param[in] zoom viewing angle of the camera, in degrees.
	 */
	void update(
	    const vec3& position,
	    const vec3& target,
	    double zoom = 20.0
	) {
	    position_ = position;
	    target_ = target;
	    
	    // Viewing vector
	    Z_ = normalize(target_ - position_);
	    
	    // Horizontal direction
	    // We construct it as a vector both orthogonal
	    // to Z_ and to the vertical direction (0 0 1).
	    X_ = cross(Z_, vec3(0.0, 0.0, 1.0));
	    
	    // Vertical direction
	    Y_ = cross(Z_,X_);
	    
	    // Coordinate of the viewing plane along viewing vector
	    double zp =
 	        (double(image_height_) / 2.0) / tan(zoom * M_PI / 180.0);
	    
	    // Center of the viewing plane
	    center_ = position_+zp*Z_;
	}

	/**
	 * \brief gets the image width.
	 * \return the image width, in pixels.
	 */
	index_t image_width() const {
	    return image_width_;
	}

	/**
	 * \brief gets the image height.
	 * \return the image height, in pixels.
	 */
	index_t image_height() const {
	    return image_height_;
	}

	/**
	 * \brief gets the image data.
	 * \return a raw pointer to the image data.
	 */
	const Memory::byte* image_data() const {
	    return image_.data();
	}

	/**
	 * \brief Launches a primary ray.
	 * \param[in] X , Y the pixel coordinates of the ray.
	 * \return the launched ray.
	 */
	Ray launch_ray(index_t X, index_t Y) const {
	    vec3 pixel3d = center_ +
		(double(X) - double(image_width_/2 ))*X_ +
		(double(Y) - double(image_height_/2))*Y_ ;

	    return Ray(
		position_,
		pixel3d - position_
	    );
	}

	/**
	 * \brief Sets a pixel of the image.
	 * \param[in] X , Y the pixel coordinates of the ray.
	 * \param[in] color the RGB color of the pixel, with components
	 *  in [0.0,1.0]. Components larger than 1.0 are clamped to 1.0.
	 */
	void set_pixel(index_t X, index_t Y, const vec3& color) {
	    geo_debug_assert(X < image_width_);
	    geo_debug_assert(Y < image_height_);
	    Memory::byte* pixel_base = &image_[(Y*image_width_+X)*bpp_];
	    pixel_base[0] = Memory::byte(std::min(color.x, 1.0)*255.0);
	    pixel_base[1] = Memory::byte(std::min(color.y, 1.0)*255.0);
	    pixel_base[2] = Memory::byte(std::min(color.z, 1.0)*255.0);
	    if(bpp_ == 4) {
		pixel_base[3] = 255;
	    }
	}

	/**
	 * \brief Saves the image in PPM file format.
	 * \param[in] filename the name of the file where to save the image.
	 */
	void save_image(const std::string& filename) const {
	    geo_assert(bpp_ == 3);
	    FILE* f = fopen(filename.c_str(),"wb");
	    if(f == nullptr) {
		std::cerr << "Could not create file: " << filename << std::endl;
		return;
	    }
	    fprintf(
		f,"P6 %d %d %d ", int(image_width_), int(image_height_), 255
	    );
	    fwrite(image_.data(), 1, image_.size(), f);
	    fclose(f);
	}
	
    private:
	vec3 position_;
	vec3 target_;
	vec3 center_;
	vec3 X_;
	vec3 Y_;
	vec3 Z_;
	index_t image_width_;
	index_t image_height_;
	vector<Memory::byte> image_;
	index_t bpp_;
    };

    /*******************************************************************/

    /**
     * \brief A material.
     */
    struct Material {
	vec3 Kd; /**< Diffuse    */
	vec3 Kr; /**< Reflection */
	vec3 Ke; /**< Emmission  */
	Material():
	    Kd(0.7, 0.7, 0.7),
	    Kr(0.0, 0.0, 0.0),
	    Ke(0.0, 0.0, 0.0) {
	}
	/**
	 * \brief Tests whether this material is reflective.
	 * \retval true if this material has non-zero Kr, false otherwise.
	 */
	bool reflective() const {
	    return (Kr.x != 0.0 || Kr.y != 0.0 || Kr.z != 0.0);
	}
	/**
	 * \brief Tests whether this material is emissive.
	 * \retval true if this material has non-zero Ke, false otherwise.
	 */
	bool emissive() const {
	    return (Ke.x != 0.0 || Ke.y != 0.0 || Ke.z != 0.0);	    
	}
    };

    /*******************************************************************/

    /**
     * \brief A Ray-Object intersection.
     */
    struct Intersection {
	/**
	 * \brief Intersection default constructor.
	 */
	Intersection() :
	    t(Numeric::max_float64()),
	    object(nullptr),
	    K(0.1, 0.1, 0.1) {
	}
	vec3 position; /**< position of the intersection. */
	vec3 normal; /**< normal to the object. */
	double t; /**< ray parameter of the intersection. */
	const Object* object; /**< intersected object. */
	vec3 K; /**< current computed ray color. */
	Material material; /**< current material. */
    };

    /*******************************************************************/

    /**
     * \brief An object that can be raytraced.
     */
    class Object {
    public:

	/**
	 * \brief Object destructor.
	 */
	virtual ~Object() {}

	/**
	 * \brief Computes the intersection with a ray.
	 * \details If there is an intersection and if it is nearer
	 *  than the previous one, then replace it.
	 * \param[in] R the ray
	 * \param[in,out] I the nearest intersection along the ray.
	 */
	virtual void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const = 0;

	/**
	 * \brief Tests whether this object shadows a ray.
	 * \details This object shadows the ray R if there is an
	 *  intersection between R.origin and R.origin + R.direction.
	 *  Intersections further away than R.origin + R.direction 
	 *  are ignored.
	 * \param[in] R the ray. R.origin corresponds to a point
	 *  queried for shadow. R.origin + R.direction corresponds to
	 *  the light-source.
	 */
	virtual bool in_shadow(const Ray& R) const = 0;

	/**
	 * \brief Sets the diffuse coefficient.
	 * \param[in] K the diffuse coefficient.
	 * \return a pointer to the Object, to allow chaining operations.
	 */
	Object* set_diffuse_coefficient(const vec3& K) {
	    material_.Kd = K;
	    return this;
	}

	/**
	 * \brief Sets the reflection coefficient.
	 * \param[in] K the reflection coefficient.
	 * \return a pointer to the Object, to allow chaining operations.
	 */
	Object* set_reflection_coefficient(const vec3& K) {
	    material_.Kr = K;
	    return this;
	}

	/**
	 * \brief Sets the emission coefficient.
	 * \param[in] K the emission coefficient.
	 * \return a pointer to the Object, to allow chaining operations.
	 */
	Object* set_emission_coefficient(const vec3& K) {
	    material_.Ke = K;
	    return this;
	}

	/**
	 * \brief Gets the Material.
	 * \return a const reference to the Material.
	 */
	const Material& material() const {
	    return material_;
	}

	/**
	 * \brief Gets the Material.
	 * \return a modifiable reference to the Material.
	 */
	Material& material() {
	    return material_;
	}
	
	Object* rename(const std::string& name) {
	    name_ = name;
	    return this;
	}

	const std::string& name() const {
	    return name_;
	}

#ifdef RAYTRACE_GUI

	bool edit_color(const std::string& name, vec3& K) {
	    float Kf[3];
	    Kf[0] = float(K.x);
	    Kf[1] = float(K.y);
	    Kf[2] = float(K.z);
	    bool result = ImGui::ColorEdit3WithPalette(name.c_str(), Kf);
	    if(result) {
		K.x = double(Kf[0]);
		K.y = double(Kf[1]);
		K.z = double(Kf[2]);		
	    }
	    return result;
	}

	bool edit_vector(const std::string& name, vec3& V) {
	    float Vf[3];
	    Vf[0] = float(V.x);
	    Vf[1] = float(V.y);
	    Vf[2] = float(V.z);
	    ImGui::SetNextItemWidth(-ImGui::CalcTextSize(name.c_str()).x);
	    bool result = ImGui::DragFloat3(
		name.c_str(), Vf, 0.1f, 0.0f, 0.0f, "%.3f"
	    );
	    if(result) {
		V.x = double(Vf[0]);
		V.y = double(Vf[1]);
		V.z = double(Vf[2]);		
	    }
	    return result;
	}

	bool edit_scalar(const std::string& name, double& V) {
	    double zero = 0.0;
	    ImGui::SetNextItemWidth(-ImGui::CalcTextSize(name.c_str()).x);
	    return ImGui::DragScalar(
		name.c_str(),
		ImGuiDataType_Double,
		&V,
		0.005f,
		&zero,
		nullptr,
		"%.3f"
	    );
	}
	
	/**
	 * \brief Draws and handle the GUI.
	 * \retval true if an element was changed.
	 * \retval false otherwise.
	 */
	virtual bool draw_gui() {
	    ImGui::PushID(this);
	    bool result = false;
	    ImGui::Separator();
	    if(ImGui::Button("X")) {
		to_delete_ = this;
	    }
	    ImGui::SameLine();
	    ImGui::Text("%s", name().c_str());
	    if(edit_color("Diffuse", material().Kd)) {
		result = true;
	    }
	    if(edit_color("Reflect", material().Kr)) {
		result = true;
	    }
	    ImGui::PopID();	    
	    return result;
	}
#endif	
	
     protected:
	std::string name_;
	Material material_;
	static Object* to_delete_;
    };

    Object* Object::to_delete_ = nullptr;
    
    /*******************************************************************/

    /**
     * \brief A sphere object.
     */
    class Sphere : public Object {
    public:

	/**
	 * \brief Sphere constructor.
	 * \param[in] center the center of the sphere.
	 * \param[in] radius the radius of the sphere.
	 */
	Sphere(const vec3& center, double radius) :
	    center_(center), radius_(radius) {
	}

        /**
	 * \brief Gets the center.
	 * \return the center of the sphere.
	 */	       
	const vec3& center() const {
	    return center_;
	}

        /**
	 * \brief Gets the radius.
	 * \return the radius of the sphere.
	 */	       
	double radius() const {
	    return radius_;
	}

        /**
	 * \copydoc Object::get_nearest_intersection()
	 */	       
	void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const override {
	    double t = get_intersection_t(R);
	    if(t > epsilon_t && t < I.t) {
		I.t = t;
		I.object = this;
		I.material = material_;
		I.position = R.origin + t * R.direction;
		I.normal = normalize(I.position - center_);
	    }
	}

        /**
	 * \copydoc Object::in_shadow()
	 */	       
	bool in_shadow(const Ray& R) const override {
	    double t = get_intersection_t(R);
	    return (t > 0.0 && t < 1.0);
	}

#ifdef RAYTRACE_GUI	
	/**
	 * \copydoc Object::draw_gui()
	 */
	 bool draw_gui() override {
	    ImGui::PushID(this);
	    bool result = Object::draw_gui();
	    if(edit_vector("C", center_)) {
		result = true;
	    }
	    if(edit_scalar("R", radius_)) {
		result = true;
	    }
	    ImGui::PopID();	    
	    return result;
	}
#endif

	
    protected:

	/**
	 * \brief Gets the coordinate of the intersection between 
	 *   this sphere and a ray.
	 * \return the coordinate of the intersection along \p R
	 *   or a negative number if there is no intersection.
	 */
	double get_intersection_t(const Ray& R) const {
	    // Detail of the computation:
	    // M = O + tD                        (1) (parametric ray eqn)
	    // (M-C)^2 = M^2 - 2M.C + C^2 = R^2  (2) (implicit sphere eqn)
	    //(O + tD)^2 - 2(O+tD).C + C^2 = R^2     (inject (1) into (2)
	    // O^2 + 2tO.D + t^2D^2 -2O.C -2tD.C + C^2 = R^2
	    // t^2 (D^2) + 2t D.(O-C) + O^2 - 2 O.C + C^2 - R^2 = 0
	    // t^2 (D^2) + 2t D.(O-C) + (O-C)^2 - R^2 = 0
	    // This is a quadratic equation in t (a t^2 + b t + c = 0),
	    // let us solve it for t now !

	    double t = -1.0;
	    
	    vec3 CO = R.origin - center_;
	    double a = length2(R.direction);
	    double b = 2.0*dot(R.direction,CO);
	    double c = length2(CO) - radius_*radius_;
	    double delta = b*b - 4.0 * a * c;
	    
	    if(delta < 0.0) {
		return -1.0;
	    }
	    double sqrt_delta = sqrt(delta);
	    t = (-b-sqrt_delta) / (2.0 * a);
	    if(t > 0) {
		return t;
	    } 
	    t = (-b+sqrt_delta) / (2.0 * a);
	    return t;
	}

	
    protected:
	vec3 center_;
	double radius_;
    };

    /*******************************************************************/

    /**
     * \brief Light object.
     * \details A Light appears as a colored sphere. The radius does not play
     *  a role in the lighting, this is just a point light.
     */
    class Light : public Sphere {
    public:
	/**
	 * \brief Light constructor.
	 * \param[in] center the position of the light.
	 * \param[in] R the radius.
	 * \param[in] K the color of the light.
	 */
	Light(const vec3& center, double R, const vec3& K) : Sphere(center, R) {
	    material_.Kd = vec3(0.0, 0.0, 0.0);
	    material_.Kr = vec3(0.0, 0.0, 0.0);
	    material_.Ke = K;
	    on_ = true;
	}

	bool on() const {
	    return on_;
	}


        /**
	 * \copydoc Object::get_nearest_intersection()
	 */	       
	void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const override {
	    Sphere::get_nearest_intersection(R,I);
	    if(!on()) {
		I.material.Ke = vec3(0.0, 0.0, 0.0);
	    }
	}
	
#ifdef RAYTRACE_GUI	
	/**
	 * \copydoc Object::draw_gui()
	 */
	 bool draw_gui() override {
	    ImGui::PushID(this);
	    bool result = false;
	    ImGui::Separator();
	    if(ImGui::Button("X")) {
		to_delete_ = this;
	    }
	    ImGui::SameLine();
	    ImGui::Text("%s", name().c_str());
	    if(ImGui::Checkbox("##On", &on_)) {
		result = true;
	    }
	    ImGui::SameLine();
	    if(edit_color("Emit.", material().Ke)) {
		result = true;
	    }
	    if(edit_vector("C", center_)) {
		result = true;
	    }
	    if(edit_scalar("R", radius_)) {
		result = true;
	    }
	    ImGui::PopID();	    
	    return result;
	}
#endif
	 
      private:
	 bool on_;
	 
    };
    
    /*******************************************************************/

    /**
     * \brief Mesh object.
     * \details Optimized ray-mesh intersections computed using an axis-aligned
     *  bounding box tree (geogram's MeshAABB).
     */
    class MeshObject : public Object {
    public:
	/**
	 * \brief MeshObject constructor.
	 * \param[in,out] M a reference to the mesh. Note that 
	 *  MeshAABB changes the order of the mesh elements.
	 */
	MeshObject(Mesh& M) : AABB_(M) {
	}

	/**
	 * \copydoc Object::get_nearest_intersection()
	 */
	void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const override {
	    // Multiply by big constant because AABB has a segment isect routine
	    // (not ray isect routine), so it would ignore intersections further
	    // away than (R.origin + R.direction), and we want them !
	    vec3 p2 = R.origin + 10000.0 * R.direction;
	    double t;
	    index_t f;
	    if(AABB_.segment_nearest_intersection(R.origin, p2, t, f)) {
		// Do not forget to take the 10000.0 factor into account else
		// the computed t does not make sense !
		t *= 10000.0;
		if(t > epsilon_t && t < I.t) {
		    I.t = t;
		    I.object = this;
		    I.material = material_;
		    I.position = R.origin + t * R.direction;
		    I.normal = normalize(
			Geom::mesh_facet_normal(*AABB_.mesh(),f)
		    );
		}
	    }
	}

	/**
	 * \copydoc Object::in_shadow()
	 */
	bool in_shadow(const Ray& R) const override {
	    vec3 p2 = R.origin + R.direction;
	    return AABB_.segment_intersection(R.origin, p2);
	}
	
    private:
	MeshFacetsAABB AABB_;
    };

    /*******************************************************************/

    /**
     * \brief The traditional checkerboard.
     * \details Cannot avoid to have this in a raytracer 
     *   (this is the tradition).
     */
    class HorizontalCheckerboardPlane : public Object {
    public:

	/**
	 * \brief HorizontalCheckerboardPlane constructor;
	 * \param[in] z altitude of the plane.
	 */
	HorizontalCheckerboardPlane(double z) : Z_(z) {
	}

	/**
	 * \copydoc Object::get_nearest_intersection()
	 */
	void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const override {
	    if(R.direction.z != 0.0) {
		double t = (Z_ - R.origin.z) / R.direction.z;
		if(t > epsilon_t && t < I.t) {
		    I.t = t;
		    I.position = R.origin + t * R.direction;
		    int X = int((I.position.x + 1000.0)* 2.0);
		    int Y = int((I.position.y + 1000.0)* 2.0);
		    double color = (((X&1) ^ (Y&1)) == 0) ? 0.0 : 1.0;
		    I.object = this;
		    I.material = material_;
		    I.material.Kd.x *= color;
		    I.material.Kd.y *= color;
		    I.material.Kd.z *= color;		    
		    I.normal = vec3(0.0, 0.0, 1.0);
		}
	    }
	}

	/**
	 * \copydoc Object::in_shadow()
	 */
	bool in_shadow(const Ray& R) const override {
	    if(R.direction.z == 0.0) {
		return false;
	    }
	    double t = (Z_ - R.origin.z) / R.direction.z;
	    return (t >= 0.0 && t <= 1.0);
	}

    private:
	double Z_;
    };

    /*******************************************************************/

    /**
     * \brief A scene to be ray-traced.
     */
    class Scene : public Object {
    public:

	/**
	 * \brief Scene destructor.
	 */
	~Scene() override {
	    for(index_t i=0; i<objects_.size(); ++i) {
		delete objects_[i];
	    }
	}

	/**
	 * \brief Adds an object to the scene.
	 * \param[in] O a pointer to the object to be added.
	 *  Pointer ownership is transfered to this Scene.
	 */
	Object* add_object(Object* O) {
	    objects_.push_back(O);
	    Light* L = dynamic_cast<Light*>(O);
	    if(L == nullptr) {
		real_objects_.push_back(O);
	    } else {
		lights_.push_back(L);
	    }
	    if(O->name() == "") {
		O->rename("object " + String::to_string(objects_.size()));
	    }
	    return O;
	}

	/**
	 * \copydoc Object::get_nearest_intersection()
	 */
	void get_nearest_intersection(
	    const Ray& R, Intersection& I
	) const override {
	    for(index_t i=0; i<objects_.size(); ++i) {
		objects_[i]->get_nearest_intersection(R,I);
	    }
	}

	/**
	 * \copydoc Object::in_shadow()
	 */
	bool in_shadow(const Ray& R) const override {
	    for(index_t i=0; i<real_objects_.size(); ++i) {
		if(real_objects_[i]->in_shadow(R)) {
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * \brief Computes the lighting at an intersection.
	 * \details Launches shadow rays to the light sources.
	 * \param[in,out] I a reference to the intersection.
	 */
	void compute_lighting(Intersection& I) const {
	    if(I.material.emissive()) {
		I.K = I.material.Ke;
	    } else {
		for(index_t i=0; i<lights_.size(); ++i) {
	            if(!lights_[i]->on()) {
	               continue;
                    }
		    vec3 L = lights_[i]->center() - I.position;
		    if(!in_shadow(Ray(I.position + epsilon_t*L, L))) {
			double Lambert = dot(I.normal, L);
			if(Lambert > 0.0) {
			    Lambert /= length(L);
			    I.K += Lambert*mul(
				I.material.Kd,lights_[i]->material().Ke
			    );
			}
		    }
		}
	    }
	}

	/**
	 * \brief Launches a ray and computes the color.
	 * \details Reflected rays are recursively computed.
	 * \param[in] R the ray to be launched.
	 * \return the computed color.
	 */
	vec3 raytrace(const Ray& R, index_t level=0) const {
	    Intersection I;
	    get_nearest_intersection(R,I);
	    if(I.object != nullptr) {
		compute_lighting(I);
		if(I.material.reflective() && level < 3) {
		    vec3 D = R.direction;
		    Ray Reflected(
			I.position,
			D - 2.0*dot(D, I.normal)*I.normal
		    );
		    vec3 Kreflect = raytrace(Reflected,level+1);
		    I.K += mul(I.material.Kr, Kreflect);
		}
	    }
	    return I.K;
	}

#ifdef RAYTRACE_GUI
	/**
	 * \copydoc Object::draw_gui()
	 */
	virtual bool draw_gui() override {
	    bool result = false;
	    
	    for(index_t i=0; i<objects_.size(); ++i) {
		if(to_delete_ == objects_[i]) {
		    delete objects_[i];
		    objects_.erase(objects_.begin() + std::ptrdiff_t(i));
		    result = true;
		    break;
		} 		
	    }

	    for(index_t i=0; i<real_objects_.size(); ++i) {
		if(to_delete_ == real_objects_[i]) {
		    real_objects_.erase(
			real_objects_.begin() + std::ptrdiff_t(i)
		    );
		    break;
		} 		
	    }

	    for(index_t i=0; i<lights_.size(); ++i) {
		if(to_delete_ == lights_[i]) {
		    lights_.erase(lights_.begin() + std::ptrdiff_t(i));
		    break;
		} 		
	    }

	    to_delete_ = nullptr;
	    
	    for(index_t i=0; i<objects_.size(); ++i) {
		if(objects_[i]->draw_gui()) {
		    result = true;
		}
	    }
	    return result;
	}
#endif	
	
    private:
	vector<Object*> objects_; /**< all the objects. */
	vector<Object*> real_objects_; /**< all the objects but the lights. */
	vector<Light*> lights_; /**< the lights are here. */
    };

    /*******************************************************************/
    /** Utilities                                                      */
    /*******************************************************************/

    /**
     * \brief Sets a 4x4 homogeneous transform matrix from a translation
     *  and a quaternion.
     * \param[out] M the matrix.
     * \param[in] Tx , Ty , Tz the translation.
     * \param[in] Qx , Qy , Qz , Qw the quaternion.
     */
    inline void set_mat4_from_translation_and_quaternion(
	mat4& M,
	double Tx, double Ty, double Tz,
	double Qx, double Qy, double Qz, double Qw
    ) {
	// for unit q, just set s = 2 or set xs = Qx + Qx, etc. 
	double s = 2.0 / (Qx*Qx + Qy*Qy + Qz*Qz + Qw*Qw);
	
	double xs = Qx * s;
	double ys = Qy * s;
	double zs = Qz * s;
	    
	double wx = Qw * xs;
	double wy = Qw * ys;
	double wz = Qw * zs;
	
	double xx = Qx * xs;
	double xy = Qx * ys;
	double xz = Qx * zs;
	
	double yy = Qy * ys;
	double yz = Qy * zs;
	double zz = Qz * zs;
	
	M(0,0) = 1.0 - (yy + zz);
	M(0,1) = xy - wz;
	M(0,2) = xz + wy;
	M(0,3) = 0.0;
	
	M(1,0) = xy + wz;
	M(1,1) = 1 - (xx + zz);
	M(1,2) = yz - wx;
	M(1,3) = 0.0;
	
	M(2,0) = xz - wy;
	M(2,1) = yz + wx;
	M(2,2) = 1 - (xx + yy);
	M(2,3) = 0.0;
	
	M(3,0) = Tx;
	M(3,1) = Ty;
	M(3,2) = Tz;
	M(3,3) = 1.0;
    }

    /**
     * \brief Transform a 3d point by an affine transform
     *  stored in a homogeneous 4x4 matrix.
     * \param[in] v the 3d point to be transformed.
     * \param[in] m the 4x4 matrix.
     * \return the transformed point.
     */
    inline vec3 transform_point(
        const vec3& v,
        const mat4& m
    ){
        index_t i,j ;
        double result[4] ;
        
        for(i=0; i<4; i++) {
            result[i] = 0 ;
        }
        for(i=0; i<4; i++) {
            for(j=0; j<3; j++) {
                result[i] += v[j] * m(j,i) ;
            }
            result[i] += m(3,i);
        }
    
        return vec3(
            result[0] / result[3],
            result[1] / result[3],
            result[2] / result[3] 
        ) ; 
    }
    
   /**
    * \brief Normalizes the coordinates of a mesh
    *  in the unit box.
    * \details A uniform scaling is applied.
    * \param[in,out] M the mesh to be normalized.
    */
    inline void normalize_mesh(Mesh& M) {
	double mesh_xyz_min[3];
	double mesh_xyz_max[3];
	get_bbox(M, mesh_xyz_min, mesh_xyz_max);
	double dim[3];
	dim[0] = (mesh_xyz_max[0] - mesh_xyz_min[0]);
	dim[1] = (mesh_xyz_max[1] - mesh_xyz_min[1]);
	dim[2] = (mesh_xyz_max[2] - mesh_xyz_min[2]);
	double max_dim = std::max(dim[0], std::max(dim[1], dim[2]));
	double s = 1.0 / max_dim;
	double T[3];
	T[0] = 0.5 * (max_dim - dim[0]);
	T[1] = 0.5 * (max_dim - dim[1]);
	T[2] = 0.5 * (max_dim - dim[2]);        
	for(index_t v=0; v<M.vertices.nb(); ++v) {
	    double* p = M.vertices.point_ptr(v);
	    double x = s * (T[0] + p[0] - mesh_xyz_min[0]);
	    double y = s * (T[1] + p[1] - mesh_xyz_min[1]);
	    double z = s * (T[2] + p[2] - mesh_xyz_min[2]);
	    p[0] = x;
	    p[1] = z;
	    p[2] = 1.0-y;
	}
    }

    inline vec3 random_color() {
	vec3 result;
	while(length2(result) < 0.1) {
	    result = vec3(
		Numeric::random_float64(),
		Numeric::random_float64(),
		Numeric::random_float64()
	    );
	}
	return result;
    }
}


#endif
