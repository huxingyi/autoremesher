/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
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

////////////////////////////////////////////////////////////////////////////////

#include "mesh_primitives.h"
#include "topopt.h"
#include "layout.h"
#include <geogram_gfx/glup_viewer/glup_viewer.h>
#include <geogram_gfx/glup_viewer/glup_viewer_gui.h>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <chrono>
#include <mutex>
#include <thread>
#include <map>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/threading.h>
#endif

////////////////////////////////////////////////////////////////////////////////
namespace {
using namespace GEO;
using namespace Eigen;
////////////////////////////////////////////////////////////////////////////////

TopoptProblem topopt;

class TopoptDemoApplication;
TopoptDemoApplication *current_app;

// -----------------------------------------------------------------------------

GLboolean mouse(float x, float y, int button, enum GlupViewerEvent event);
void one_step_topopt();
void clear_design();

// -----------------------------------------------------------------------------

class TopoptDemoApplication : public GEO::SimpleMeshApplication {
public:
	TopoptDemoApplication(int argc, char** argv)
		: SimpleMeshApplication(argc, argv, "")
		, design_need_refresh_(false)
		, edit_forces_(false)
		, edit_fixed_(false)
		, fixed_brush_size_(1)
		, closest_orig_({nullptr, 0})
		, closest_endpoint_({nullptr, 0})
		, last_button_(-1)
		, topopt_solver_(topopt)
		, time_last_iter_(std::chrono::steady_clock::now())
		, topopt_thread_please_stop_(false)
	{
		name_ = "[Float] Topopt demo";
		this->lighting_ = true;
		this->white_bg_ = true;
		this->left_pane_visible_ = true;
		this->right_pane_visible_ = true;
		this->console_visible_ = false;
		this->show_mesh_ = false;
		glup_viewer_set_screen_size(1024, 800);
		glup_viewer_set_mouse_func(::mouse);
		init_mesh();
		current_app = this;
	}

	~TopoptDemoApplication() {
		// Wait for topopt thread to finish
		topopt_thread_please_stop_ = true;
		for (std::thread &t : topopt_thread_) {
			if (t.joinable()) { t.join(); }
		}
	}

	void init_mesh() {
		mesh_gfx()->set_mesh(nullptr);
		mesh()->clear(false, false);

		// Create the mesh of regular grid
		GEO::Mesh &M = *mesh();
		Vector2i nnodes = topopt.nelems + Vector2i(1, 1);
		M.vertices.create_vertices(nnodes.prod());
		for (int idx = 0; idx < nnodes.prod(); ++idx) {
			Vector2i pos = Layout::to_grid(idx, nnodes);
			M.vertices.point(idx) = vec3(pos[0], pos[1], 0);
		}

		index_t first_quad = M.facets.create_quads(topopt.nelems.prod());
		for (index_t q = 0; q < topopt.nelems.prod(); ++q) {
			Vector2i min_corner = Layout::to_grid(q, topopt.nelems);
			Vector2i diff[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
			for (index_t lv = 0; lv < 4; ++lv) {
				int v = Layout::to_index(min_corner + diff[lv], nnodes);
				M.facets.set_vertex(first_quad + q, lv, v);
			}
		}
		M.facets.connect();

		// Administrative stuff
		mesh_gfx()->set_animate(false);
		double xyzmin[3];
		double xyzmax[3];
		get_bbox(*mesh(), xyzmin, xyzmax, false);
		glup_viewer_set_region_of_interest(
			float(xyzmin[0]), float(xyzmin[1]), float(xyzmin[2]),
			float(xyzmax[0]), float(xyzmax[1]), float(xyzmax[2])
		);

		mesh_gfx()->set_mesh(mesh());

		// Init arrow mesh
		create_arrow_2d(0.5, 1, 3.5, 1.5, arrow_mesh_);
		arrow_mesh_gfx_.set_mesh(&arrow_mesh_);

		refresh_nnz_forces();
	}

	void refresh_nnz_forces() {
		nonzero_forces_.clear();
		for (int v = 0; v < mesh()->vertices.nb(); ++v) {
			vec2 f(topopt.forces(2*v), topopt.forces(2*v+1));
			if (f[0] != 0 || f[1] != 0) {
				nonzero_forces_.push_back(v);
			}
		}
	}

	void refresh_densities() {
		std::lock_guard<std::mutex> lck(mtx_densities_gfx_);
		Attribute<float> densities(mesh()->facets.attributes(), "densities");
		for (int f = 0; f < mesh()->facets.nb(); ++f) {
			densities[f] = topopt.densities(f);
		}
	}

	void reset_design() {
		std::lock_guard<std::mutex> lck(mtx_densities_top_);
		topopt_solver_.resetDesign();
		refresh_densities();
	}

	void rescale_design() {
		std::lock_guard<std::mutex> lck(mtx_densities_top_);
		topopt_solver_.rescaleDesign(
			std::max(0.0, topopt.volfrac - 0.1),
			std::min(1.0, topopt.volfrac + 0.1));
		refresh_densities();
		design_need_refresh_ = false;
	}

	void one_step_topopt() {
		std::lock_guard<std::mutex> lck(mtx_densities_top_);
		topopt_solver_.iter();
		refresh_densities();
	}

	void animate_topopt() {
		#ifdef __EMSCRIPTEN__
		if (!emscripten_has_threading_support()) {
			auto time_now = std::chrono::steady_clock::now();
			auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_last_iter_);
			if (time_span.count() > 100) {
				one_step_topopt();
				time_last_iter_ = std::chrono::steady_clock::now();
			}
			return;
		}
		#endif
		if (topopt_thread_.empty()) {
			// Init topopt thread if needed
			topopt_thread_.emplace_back([this]() {
				while (!topopt_thread_please_stop_) {
					if (glup_viewer_is_enabled(GLUP_VIEWER_IDLE_REDRAW)) {
						this->one_step_topopt();
						std::this_thread::yield();
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			});
		}
	}

	virtual void init_graphics() {
		SimpleMeshApplication::init_graphics();
		glup_viewer_disable(GLUP_VIEWER_BACKGROUND);
		glup_viewer_disable(GLUP_VIEWER_3D);
		glup_viewer_add_toggle('f', &edit_forces_, "edit forces");
		glup_viewer_add_toggle('n', &edit_fixed_, "edit fixed nodes");
		glup_viewer_add_key_func('u', ::one_step_topopt, "one iteration");
		glup_viewer_add_key_func('c', ::clear_design, "clear design");

		// Attributes
		refresh_densities();
		this->set_attribute("facets.densities");
		this->attribute_min_ = 0.0;
		this->attribute_max_ = 1.0;
		this->current_colormap_texture_ = colormaps_[3].texture;
		this->show_attributes();

		// Fixed nodes
		Attribute<bool> fixed(mesh()->vertices.attributes(), "fixed");
		for (int v = 0; v < mesh()->vertices.nb(); ++v) {
			fixed[v] = (topopt.fixed(2*v) > 0) || (topopt.fixed(2*v+1) > 0);
		}
	}

	virtual void draw_scene() {
		SimpleMeshApplication::draw_scene();
		if (show_vertices_selection_) {
			mesh_gfx_.set_points_color(1.0, 0.0, 0.0);
			mesh_gfx_.set_points_size(vertices_size_);
			mesh_gfx_.set_vertices_selection("fixed");
			mesh_gfx_.draw_vertices();
			mesh_gfx_.set_vertices_selection("");
		}

		// Draw forces
		for (int v : nonzero_forces_) {
			vec2 f(topopt.forces(2*v), topopt.forces(2*v+1));
			double alpha = std::atan2(f[1], f[0]) * 180 / M_PI;
			auto pos = Layout::to_grid<vec2>(v, topopt.nelems[0]+1, topopt.nelems[1]+1);

			glupPushMatrix();
			glupTranslated(pos[0], pos[1], 0.001);
			glupRotated(alpha-90, 0, 0, 1);
			glupScaled(1, 0.2*f.length(), 1);
			arrow_mesh_gfx_.set_lighting(lighting_);
			arrow_mesh_gfx_.set_surface_color(0, 0.6f, 0);
			arrow_mesh_gfx_.set_show_mesh(false);
			arrow_mesh_gfx_.draw_surface();
			glupPopMatrix();
		}

		GLUPfloat old_point_size = glupGetPointSize();
		glupSetPointSize(vertices_size_ * 5.f);
		glupSetColor3d(GLUP_FRONT_AND_BACK_COLOR, 0, 0.4, 0);
		GLUPboolean vertex_color_was_enabled = glupIsEnabled(GLUP_VERTEX_COLORS);
		glupEnable(GLUP_VERTEX_COLORS);
		for (int v : nonzero_forces_) {
			vec2 f(topopt.forces(2*v), topopt.forces(2*v+1));
			auto pos = Layout::to_grid<vec2>(v, topopt.nelems[0]+1, topopt.nelems[1]+1);
			glupBegin(GLUP_POINTS);
			glupColor3d(0, 0, 0.4);
			glupVertex3d(pos[0], pos[1], 0.1);
			glupColor3d(0, 0.4, 0);
			glupVertex3d(pos[0]+f[0], pos[1]+f[1], 0.1);
			glupEnd();
		}
		glupSetPointSize(old_point_size);
		if (!vertex_color_was_enabled) {
			glupDisable(GLUP_VERTEX_COLORS);
		}
		if (design_need_refresh_ && last_button_ == -1) {
			rescale_design();
		}
		if (glup_viewer_is_enabled(GLUP_VIEWER_IDLE_REDRAW)) {
			animate_topopt();
		}
	}

	virtual void draw_left_pane() {
		int w, h;
		glup_viewer_get_screen_size(&w, &h);
		if(status_bar_->active()) { h -= (STATUS_HEIGHT()+1); }
		if(console_visible_) { h -= (CONSOLE_HEIGHT()+1); }
		h -= MENU_HEIGHT();
		if(Command::current() != nullptr) { h /= 2; }

		ImGui::SetNextWindowPos(ImVec2(0.0f, float(MENU_HEIGHT())), ImGuiSetCond_Always);
		ImGui::SetNextWindowSize(ImVec2(float(PANE_WIDTH()), float(h)), ImGuiSetCond_Always);

		ImGui::Begin("Topopt", &left_pane_visible_, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Initial design");
		ImGui::Checkbox("Edit forces [f]", &edit_forces_);
		ImGui::Checkbox("Edit fixed [n]", &edit_fixed_);
		if (edit_fixed_) {
			fixed_brush_size_ = std::round(2.0 * fixed_brush_size_) / 2.0;
			ImGui::SliderInt("sz.", &fixed_brush_size_, 1, 10);
		}
		ImGui::Text("Volume fraction");
		ImGui::SliderFloat("vol.", &topopt.volfrac, 0.f, 1.f, "%.1f");
		ImGui::Text("Filter radius");
		ImGui::SliderFloat("rad.", &topopt.radius, 1.f, 10.f, "%.1f");
		ImGui::Text("Filter type");
		std::map<FilterType, std::string> names = {
			{FilterType::NoFilter, "No Filter"},
			{FilterType::Densities, "Densities"},
			{FilterType::Sensitivies, "Sensitivies"}
		};
		if (ImGui::Button((names[topopt.filter] + "##FilterType").c_str(), ImVec2(-1, 0))) {
			ImGui::OpenPopup("##FilterType");
		}
		if(ImGui::BeginPopup("##FilterType")) {
			FilterType filter_type[3] = {FilterType::NoFilter, FilterType::Densities, FilterType::Sensitivies};
			std::string filter_name[3] = {"No filter", "Densities", "Sensitivies"};
			for (int i : {0, 1, 2}) {
				if (ImGui::Button(filter_name[i].c_str())) {
					topopt.filter = filter_type[i];
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}

		ImGui::Separator();
		ImGui::Text("Design update");
		if (ImGui::Button("One iteration [u]", ImVec2(-1, 0))) {
			one_step_topopt();
		}
		ImGui::Checkbox("animate [a]", (bool*)glup_viewer_is_enabled_ptr(GLUP_VIEWER_IDLE_REDRAW));

		ImGui::Separator();
		if (ImGui::Button("Reset design [c]", ImVec2(-1, 0))) {
			reset_design();
		}

		ImGui::End();
	}

	virtual void draw_console() {
		this->console_visible_ = false;
	}

	virtual void draw_object_properties() {
		ImGui::Checkbox("attributes", &show_attributes_);
		if(show_attributes_) {
			if(attribute_min_ == 0.0f && attribute_max_ == 0.0f) {
				autorange();
			}
			if(ImGui::Button(
				   (attribute_ + "##Attribute").c_str(), ImVec2(-1,0))
			) {
				ImGui::OpenPopup("##Attributes");
			}
			if(ImGui::BeginPopup("##Attributes")) {
				std::vector<std::string> attributes;
				String::split_string(attribute_names(), ';', attributes);
				for(index_t i=0; i<attributes.size(); ++i) {
					if(ImGui::Button(attributes[i].c_str())) {
						set_attribute(attributes[i]);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
			ImGui::InputFloat("min",&attribute_min_);
			ImGui::InputFloat("max",&attribute_max_);
			if(ImGui::ImageButton(
				   convert_to_ImTextureID(current_colormap_texture_),
				   ImVec2(115.0f*scaling(),8.0f*scaling()))
			) {
				ImGui::OpenPopup("##Colormap");
			}
			if(ImGui::BeginPopup("##Colormap")) {
				for(index_t i=0; i<colormaps_.size(); ++i) {
					if(ImGui::ImageButton(
						   convert_to_ImTextureID(colormaps_[i].texture),
						   ImVec2(100.0f*scaling(),8.0f*scaling()))
					) {
						current_colormap_texture_ = colormaps_[i].texture;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}

		if(mesh_.vertices.dimension() >= 6) {
			ImGui::Separator();
			ImGui::Checkbox(
				"Animate [a]",
				(bool*)glup_viewer_is_enabled_ptr(GLUP_VIEWER_IDLE_REDRAW)
			);
			ImGui::SliderFloat("spd.", &anim_speed_, 1.0f, 10.0f, "%.1f");
			ImGui::SliderFloat("t.", &anim_time_, 0.0f, 1.0f, "%.2f");
		}

		ImGui::Separator();
		ImGui::Checkbox("Fixed nodes", &show_vertices_selection_);
		ImGui::SliderFloat("sz.", &vertices_size_, 0.1f, 5.0f, "%.1f");

		if(mesh_.facets.nb() != 0) {
			ImGui::Separator();
			ImGui::Checkbox("Surface [S]", &show_surface_);
			if(show_surface_) {
				ImGui::Checkbox("colors [c]", &show_surface_colors_);
				ImGui::Checkbox("mesh [m]", &show_mesh_);
				ImGui::Checkbox("borders [B]", &show_surface_borders_);
			}
		}

		if(mesh_.cells.nb() != 0) {
			ImGui::Separator();
			ImGui::Checkbox("Volume [V]", &show_volume_);
			if(show_volume_) {
				ImGui::SliderFloat(
					"shrk.", &cells_shrink_, 0.0f, 1.0f, "%.2f"
				);
				if(!mesh_.cells.are_simplices()) {
					ImGui::Checkbox("colored cells [C]", &show_colored_cells_);
					ImGui::Checkbox("hexes [j]", &show_hexes_);
				}
			}
		}
	}

	GLboolean mouse(float x, float y, int button, enum GlupViewerEvent event) {
		GLdouble xyz[3];
		GLboolean bkgnd;
		glup_viewer_get_picked_point(xyz, &bkgnd);
		vec2 p(xyz[0], xyz[1]);

		switch(event) {
		case GLUP_VIEWER_DOWN:
			last_button_ = button;
			break;
		case GLUP_VIEWER_UP:
			last_button_ = -1;
			break;
		case GLUP_VIEWER_MOVE:
			break;
		}

		if (edit_fixed_ && (last_button_ == GLFW_MOUSE_BUTTON_1 || last_button_ == GLFW_MOUSE_BUTTON_2)) {
			int x(std::round(p[0]));
			int y(std::round(p[1]));

			bool new_state = (last_button_ == GLFW_MOUSE_BUTTON_1);

			if (x >= 0 && y >= 0 && x <= topopt.nelems[0]+1 && y <= topopt.nelems[1]+1) {
				Attribute<bool> fixed(mesh()->vertices.attributes(), "fixed");
				Vector2i min_corner = Vector2i(x, y) - (fixed_brush_size_-1) * Vector2i::Ones();
				Vector2i max_corner = Vector2i(x, y) + (fixed_brush_size_-1) * Vector2i::Ones();
				min_corner = min_corner.cwiseMax(0);
				max_corner = max_corner.cwiseMin(topopt.nelems);
				for (int xx = min_corner[0]; xx <= max_corner[0]; ++xx) {
					for (int yy = min_corner[1]; yy <= max_corner[1]; ++yy) {
						if (Vector2i(xx-x, yy-y).squaredNorm() <= fixed_brush_size_) {
							int v = Layout::to_index(xx, yy, topopt.nelems[0]+1, topopt.nelems[1]+1);
							fixed[v] = new_state;
							topopt.fixed.segment<2>(2*v).setConstant(new_state ? 1.0 : 0.0);
							design_need_refresh_ = true;
						}
					}
				}
			}

			return GL_TRUE;
		}

		if (edit_forces_) {
			//////////////////////
			// Helper functions //
			//////////////////////

			auto sq_dist_force = [&](int v) {
				Vector2d a = Layout::to_grid<Vector2i>(v, topopt.nelems + Vector2i(1, 1)).cast<double>();
				Vector2d b = a + topopt.forces.segment<2>(2*v);
				double dist_a = (a - Vector2d(p[0], p[1])).squaredNorm();
				double dist_b = (b - Vector2d(p[0], p[1])).squaredNorm();
				return std::make_pair(dist_a, dist_b);
			};

			auto closest_grid_node_index = [&]() {
				Vector2d node(p[0], p[1]);
				node = node.unaryExpr([] (double x) { return std::round(x); });
				node = node.cwiseMax(0).cwiseMin(topopt.nelems.cast<double>());
				return Layout::to_index(node[0], node[1], topopt.nelems[0]+1, topopt.nelems[1]+1);
			};

			auto has_nonzero_forces = [&](int i) {
				for (int j : nonzero_forces_) { if (j == i) { return true; } }
				return false;
			};

			auto delta_from_grid_node = [&](int v) {
				Vector2d diff(p[0], p[1]);
				diff -= Layout::to_grid<Vector2i>(v, topopt.nelems + Vector2i(1, 1)).cast<double>();
				double radius = diff.norm();
				double alpha = std::atan2(diff[1], diff[0]);
				alpha = std::round(32.0 * alpha / (2.0 * M_PI)) / 32.0 * 2.0 * M_PI;
				diff = radius * Vector2d(std::cos(alpha), std::sin(alpha));
				return diff;
			};

			// Find closest node with nnz external force
			double max_active_dist = 0.1 * topopt.nelems.maxCoeff();
			if (event == GLUP_VIEWER_DOWN) {
				closest_orig_ = {nullptr, std::numeric_limits<double>::max()};
				closest_endpoint_ = {nullptr, std::numeric_limits<double>::max()};
				for (int &v : nonzero_forces_) {
					auto dist = sq_dist_force(v);
					if (dist.first < closest_orig_.dist) {
						closest_orig_.index = &v;
						closest_orig_.dist = dist.first;
					}
					if (dist.second < closest_endpoint_.dist) {
						closest_endpoint_.index = &v;
						closest_endpoint_.dist = dist.second;
					}
				}
			}

			if (last_button_ == GLFW_MOUSE_BUTTON_1) {
				// Edit force
				if (closest_orig_.index && closest_endpoint_.index && closest_orig_.dist < closest_endpoint_.dist) {
					if (closest_orig_.index && closest_orig_.dist < max_active_dist) {
						int &v = *closest_orig_.index;
						int i = closest_grid_node_index();
						if (!has_nonzero_forces(i)) {
							topopt.forces.segment<2>(2*i) = topopt.forces.segment<2>(2*v);
							topopt.forces.segment<2>(2*v).setZero();
							v = i;
							design_need_refresh_ = true;
						}
					}
				} else {
					if (closest_endpoint_.index && closest_endpoint_.dist < max_active_dist) {
						int v = *closest_endpoint_.index;
						topopt.forces.segment<2>(2*v) = delta_from_grid_node(v);
						design_need_refresh_ = true;
					}
				}
			} else if (last_button_ == GLFW_MOUSE_BUTTON_2) {
				// Add new force
				if (event == GLUP_VIEWER_DOWN) {
					int i = closest_grid_node_index();
					if (!has_nonzero_forces(i)) {
						topopt.forces.segment<2>(2*i) = delta_from_grid_node(i);
						refresh_nnz_forces();
						for (int &v : nonzero_forces_) { if (v == i) { closest_endpoint_.index = &v; } }
						closest_endpoint_.dist = 0;
						design_need_refresh_ = true;
					}
				} else {
					if (closest_endpoint_.index && closest_endpoint_.dist < max_active_dist) {
						int v = *closest_endpoint_.index;
						topopt.forces.segment<2>(2*v) = delta_from_grid_node(v);
						design_need_refresh_ = true;
					}
				}
			} else if (last_button_ == GLFW_MOUSE_BUTTON_3) {
				// Delete force
				if (event == GLUP_VIEWER_DOWN) {
					if (closest_orig_.index && closest_orig_.dist < max_active_dist) {
						int v = *closest_orig_.index;
						topopt.forces.segment<2>(2*v).setZero();
						refresh_nnz_forces();
						design_need_refresh_ = true;
					}
				}
			}

			return GL_TRUE;
		}

		if (event == GLUP_VIEWER_DOWN) {
			last_down_x_ = x;
			last_down_y_ = y;
		}

		return GL_FALSE;
	}

private:
	bool design_need_refresh_;
	bool edit_forces_;
	bool edit_fixed_;
	int fixed_brush_size_;
	std::vector<int> nonzero_forces_;

	// closest force endpoints
	struct { int *index; double dist; } closest_orig_, closest_endpoint_;

	int last_button_;
	float last_down_x_, last_down_y_;

	Mesh arrow_mesh_;
	MeshGfx arrow_mesh_gfx_;

	TopoptSolver topopt_solver_;
	std::chrono::steady_clock::time_point time_last_iter_;
	std::mutex mtx_densities_top_;
	std::mutex mtx_densities_gfx_;
	std::vector<std::thread> topopt_thread_;
	bool topopt_thread_please_stop_;
};

// -----------------------------------------------------------------------------

GLboolean mouse(float x, float y, int button, enum GlupViewerEvent event) {
	return current_app->mouse(x, y, button, event);
}

void one_step_topopt() {
	current_app->one_step_topopt();
};

void clear_design() {
	current_app->reset_design();
};

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	// Init topopt struct
	int nelx = 100;
	int nely = 50;
	topopt.nelems = Eigen::Vector2i(nelx, nely);
	topopt.densities.resize(nelx*nely);
	topopt.forces.resize(2*(nelx+1)*(nely+1));
	topopt.fixed.resize(2*(nelx+1)*(nely+1));
	topopt.volfrac = 0.4;
	// Densities
	topopt.densities.setConstant(topopt.volfrac);
	// External forces
	topopt.forces.setZero();
	topopt.forces(2*Layout::to_index(nelx, nely/2, nelx+1, nely+1) + 1) = -3;
	// Fixed nodes/dofs
	topopt.fixed.setZero();
	for (int y = 0; y <= nely; ++y) {
		topopt.fixed.segment<2>(2*Layout::to_index(0, y, nelx+1, nely+1)).setConstant(1);
	}

	TopoptDemoApplication app(argc, argv);
	app.start();

	return 0;
}
