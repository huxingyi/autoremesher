
#include <exploragram/basic/common.h>
#include <exploragram/hexdom/hexdom_pipeline.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/spherical_harmonics_l4.h>

#include <geogram/basic/logger.h>
#include <geogram/basic/process.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/file_system.h>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/mesh_geometry.h>

namespace {
    using namespace GEO;
    
    std::string filecode_to_filename(const std::string& filecode) {
	std::string filename = CmdLine::get_arg("hexdom:"+filecode);
	if(filecode != "tets") {
	    std::string suffix = CmdLine::get_arg("hexdom:file_suffix");
	    if(suffix != "") {
		filename = FileSystem::dir_name(filename) + "/" +
		    FileSystem::base_name(filename) + suffix + "." +
		    FileSystem::extension(filename);
	    }
	    filename = CmdLine::get_arg("hexdom:file_prefix") + filename;
	}
	if(FileSystem::extension(filename) == "geogram" && CmdLine::get_arg_bool("ASCII")) {
	    filename = filename + "_ascii";
	}
	return filename;
    }

    void load_mesh(const std::string& filecode, Mesh& M) {
	MeshIOFlags ioflags;
	ioflags.set_attributes(MESH_ALL_ATTRIBUTES);
	std::string filename = filecode_to_filename(filecode);
	if(!mesh_load(filename, M, ioflags)) {
	    throw(filecode + "(" + filename + ") : Could not load file");
	}
    }

    void save_mesh(const Mesh& M, const std::string& filecode) {
	MeshIOFlags ioflags;
	ioflags.set_attributes(MESH_ALL_ATTRIBUTES);
	std::string filename = filecode_to_filename(filecode);
	if(!mesh_save(M,filename,ioflags)) {
	    throw(filecode + "(" + filename + ") : Could not save file");    
	}
    }

    void hexdom_stage(const std::string& stage) {
	if(stage != "all") {
	    CmdLine::ui_separator(stage);
	}
	if(stage == "SetConstraints") {
	    Mesh input;
	    load_mesh("tets",input);
	    std::string message("");

	    if(CmdLine::get_arg_bool("hexdom:SetConstraints:tetrahedralize")) {
		mesh_tetrahedralize(
		    input,false,true,
		    CmdLine::get_arg_double("hexdom:SetConstraints:tet_quality")
		);
	    }
	    
	    HexdomPipeline::SetConstraints(&input, message);
	    save_mesh(input,"input");
	    if(message != "") {
		Logger::out("HexDom") << "SetConstraints message:" << message << std::endl;
	    }
	} else if(stage == "FrameField") {
	    Mesh input;
	    load_mesh("input",input);
	    HexdomPipeline::FrameField(
		  &input, CmdLine::get_arg_bool("FrameField:smooth")
	    );
	    save_mesh(input, "FF");
	} else if(stage == "Parameterization") {
	    Mesh FF;
	    load_mesh("FF", FF);
	    HexdomPipeline::Parameterization(
		 &FF,
		 CmdLine::get_arg_int("hexdom:Parameterization:algo"),
		 CmdLine::get_arg_double("hexdom:Parameterization:PGP_max_scale_corr")
	    );
	    save_mesh(FF, "uvw");
	} else if(stage == "HexCandidates") {
	    Mesh uvw;
	    Mesh hexset;
	    load_mesh("uvw", uvw);
	    HexdomPipeline::HexCandidates(&uvw, &hexset);
	    save_mesh(hexset, "hexset");
	} else if(stage == "QuadDominant") {
	    Mesh uvw;
	    Mesh quadtri;
	    load_mesh("uvw", uvw);
	    HexdomPipeline::QuadDominant(&uvw, &quadtri);
	    save_mesh(quadtri, "quadtri");
	} else if(stage == "Hexahedrons") {
	    Mesh hexset;
	    Mesh quadtri;
	    Mesh hexes;
	    load_mesh("hexset", hexset);
	    load_mesh("quadtri", quadtri);
	    HexdomPipeline::Hexahedrons(&quadtri, &hexset, &hexes);
	    save_mesh(hexes, "hexes");
	} else if(stage == "Cavity") {
	    Mesh quadtri;
	    Mesh hexes;
	    Mesh holes;
	    load_mesh("quadtri", quadtri);	    
	    load_mesh("hexes", hexes);
	    HexdomPipeline::Cavity(&quadtri, &hexes, &holes);
	    save_mesh(holes, "holes");
	} else if(stage == "HexDominant") {
	    Mesh holes;
	    Mesh hexes;
	    Mesh hexdom;
	    load_mesh("holes",holes);
	    load_mesh("hexes",hexes);
	    HexdomPipeline::HexDominant(
		&holes, &hexes, &hexdom, CmdLine::get_arg_bool("hexdom:HexDominant:with_pyramids")
	    );
	    save_mesh(hexdom, "hexdom");
	} else if(stage == "all") {
	    hexdom_stage("SetConstraints");
	    hexdom_stage("FrameField");
	    hexdom_stage("Parameterization");
	    hexdom_stage("HexCandidates");
	    hexdom_stage("QuadDominant");
	    hexdom_stage("Hexahedrons");
	    hexdom_stage("Cavity");
	    hexdom_stage("HexDominant");
	} else {
	    throw(stage + ": invalid stage");
	}
    }
    
    void show_stats(Mesh& M) {
	double total_volume=0.0;
	double hex_volume=0.0;
	double tet_volume=0.0;
	for(index_t c=0; c<M.cells.nb(); ++c) {
	    double V = mesh_cell_volume(M,c);
	    if(M.cells.type(c) == MESH_HEX) {
		hex_volume += V;
	    } else if(M.cells.type(c) == MESH_TET) {
		tet_volume += V;
	    }
	    total_volume += V;
	}
	double prop = 0.0;
	if(total_volume != 0.0) {
	    prop = 100.0 * (hex_volume / total_volume);
	}
        prop = double(int(prop * 10.0)) / 10.0; 
	Logger::out("hexdom") << "Hex proportion=" << prop << "% vol." << std::endl;
	Logger::out("hexdom") << "Total volume=" << total_volume << std::endl;
	Logger::out("hexdom") << "Tets volume=" << tet_volume << std::endl;
	Logger::out("hexdom") << "Hex volume=" << hex_volume << std::endl;	
    }
}

int main(int argc, char** argv) {
    using namespace GEO;
    GEO::initialize();

    geo_register_attribute_type<vec3i>("vec3i");
    geo_register_attribute_type<mat3>("mat3");
    geo_register_attribute_type<SphericalHarmonicL4>("SphericalHarmonicL4");

    CmdLine::import_arg_group("standard");
    CmdLine::import_arg_group("algo");    
    CmdLine::declare_arg_group("hexdom", "hex-dominant meshing pipeline");
    CmdLine::declare_arg("hexdom:stage", "all",
	"one of SetConstraints,FrameField,Parameterization,HexCandidates,\
QuadDominant,Hexahedrons,Cavity,HexDominant,all");

    CmdLine::declare_arg(
        "hexdom:file_prefix", "", "prefix prepended to all file names"
    );
    CmdLine::declare_arg(
        "hexdom:file_suffix", "",
	"suffix appended to all file base names (before extension)"
    );
    CmdLine::declare_arg(
        "hexdom:tets", "<undefined>", "the input tet mesh"
    );
    CmdLine::declare_arg(
	"hexdom:input", "input.geogram", "the tet mesh with constraints"
    );
    CmdLine::declare_arg(
        "hexdom:FF", "FF.geogram", "the frame field file"
    );
    CmdLine::declare_arg(
        "hexdom:uvw", "uvw.geogram", "the parameterized mesh file"
    );
    CmdLine::declare_arg(
        "hexdom:hexset", "hexset.geogram", "the candidate hexahedra file" 
    );
    CmdLine::declare_arg(
        "hexdom:quadtri", "quadtri.geogram", "the quad-dominant mesj file"
    );
    CmdLine::declare_arg(
        "hexdom:hexes", "hexes.geogram", "the hexahedra file"
    );
    CmdLine::declare_arg(
        "hexdom:holes", "holes.geogram", "the cavity file"
    );
    CmdLine::declare_arg(
        "hexdom:hexdom", "hexdom.geogram", "the hex dominant mesh file"	 
    );

    CmdLine::declare_arg(
        "hexdom:SetConstraints:tetrahedralize", false, "if input is surfacic, tetrahedralize it"
    );
    
    CmdLine::declare_arg(
        "hexdom:SetConstraints:tet_quality", 0.5, "tet quality (smaller=better)"
    );
    
    CmdLine::declare_arg(
        "hexdom:FrameField:smooth", false, "smooth frame field"
    );
    
    CmdLine::declare_arg(
	 "hexdom:Parameterization:algo", 0, "one of 0(PGP with corr.), 1(CubeCover), 2(PGP without corr.)"
    );

    CmdLine::declare_arg(
	 "hexdom:Parameterization:PGP_max_scale_corr", 0.3, "PGP max. scale correction"
    );

    CmdLine::declare_arg(
	"hexdom:HexDominant:with_pyramids", false, "Generate conformal mesh using pyramids"
    );

    CmdLine::declare_arg(
	"stats_only", false, "Show statistics on input mesh"
    );

    CmdLine::declare_arg("ASCII", false, "use .geogram_ascii instead of .geogram files");

    
    std::vector<std::string> filenames;
    if(!CmdLine::parse(argc, argv, filenames, "<inputfile>")) {
	return 1;
    }

    if(filenames.size() == 1) {
	CmdLine::set_arg("hexdom:tets", filenames[0]);
    }
    
    try {
	if(CmdLine::get_arg_bool("stats_only")) {
	    if(filenames.size() != 1) {
		throw("Needs an input file");
	    }
	    Mesh M;
	    if(!mesh_load(filenames[0],M)) {
		throw("Could not load file");
	    }
	    show_stats(M);
	} else {
	    hexdom_stage(CmdLine::get_arg("hexdom:stage"));
	}
    } catch(const std::string& error_string) {
	Logger::err("HexDom") << "Caught exception, message=   " << error_string
			      << std::endl;
    } catch(const std::exception& e) {
	Logger::err("HexDom") << "Caught exception, message=   " << e.what()
			      << std::endl;
    } catch(...) {
	Logger::err("HexDom") << "Caught exception" << std::endl;
    }
    
    return 0;
}

