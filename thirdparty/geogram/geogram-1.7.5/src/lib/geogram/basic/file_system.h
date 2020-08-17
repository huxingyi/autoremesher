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

#ifndef GEOGRAM_BASIC_FILE_SYSTEM
#define GEOGRAM_BASIC_FILE_SYSTEM

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>

#include <string>
#include <vector>
#include <map>

/**
 * \file geogram/basic/file_system.h
 * \brief Functions and times for filesystem manipulation
 */

namespace GEO {

    /**
     * \brief Abstraction layer for file-system management.
     */
    namespace FileSystem {

	/**
	 * \brief A Node in a FileSystem.
	 * \details This class abstracts a FileSystem and
	 *  operations on it.
	 */
	class GEOGRAM_API Node : public Counted {
	public:
	    /**
	     * \brief Node constructor.
	     */
	    Node();
	    
	    /**
	     * \brief Node destructor.
	     */
	    ~Node() override;

	    /************************** OS-dependent **************************/
	    
	    /**
	     * \brief Checks if a path is a regular file.
	     * \param[in] path system path to verify.
	     * \retval true if \p path is a regular file.
	     * \retval false otherwise.
	     */
	    virtual bool is_file(const std::string& path);

	    /**
	     * \brief Checks if a path is a directory.
	     * \param[in] path system path to verify.
	     * \retval true if \p path is a directory.
	     * \retval false otherwise.
	     */
	    virtual bool is_directory(const std::string& path);

	    /**
	     * \brief Creates a directory
	     * \details This recursively creates a new directory given by its \b
	     * absolute path \p path, creating any missing intermediate
	     * directories on the fly.
	     * \param[in] path absolute path to the directory to be created.
	     * \retval true if the directory was successfully created.
	     * \retval false otherwise.
	     */
	    virtual bool create_directory(const std::string& path);

	    /**
	     * \brief Deletes a directory
	     * \details This deletes the directory specified by path \p path. 
	     *  The path must specify an empty directory.
	     * \param[in] path the path of the directory to be removed.
	     * \retval true if the directory was successfully deleted.
	     * \retval false otherwise.
	     */
	    virtual bool delete_directory(const std::string& path);

	    /**
	     * \brief Deletes a file
	     * \param[in] path the path of the file to be deleted.
	     * \retval true if the file path was successfully deleted
	     * \retval false otherwise
	     */
	    virtual bool delete_file(const std::string& path);

	    /**
	     * \brief Lists directory contents
	     * \details Lists all the files and sub-directories in the directory
	     * specified by \p path, and stores the list in \p result. Special
	     * entries "." and ".." are not stored in \p result.
	     * \param[in] path the path to the directory to list.
	     * \param[in] result output vector of files and sub-directories.
	     * \retval true if \p path specifies a readable directory.
	     * \retval false otherwise.
	     */
	    virtual bool get_directory_entries(
		const std::string& path, std::vector<std::string>& result
	    );

	    /**
	     * \brief Gets the current working directory.
	     * \return The absolute path to the current directory.
	     */
	    virtual std::string get_current_working_directory();

	    /**
	     * \brief Sets the working directory.
	     * \param[in] path path to the new working directory.
	     * \retval true if the current directory could be 
	     *  changed to \p path.
	     * \retval false otherwise.
	     */
	    virtual bool set_current_working_directory(
		const std::string& path
	    );

	    /**
	     * \brief Renames or moves a file.
	     * \details This renames the existing file or directory specified by
	     * path \p old_name to the new path \p new_name. The new name 
	     * must not be the name of an existing file or directory. 
	     * If \p old_name and \p new_name are not in the same directory, 
	     *  \p old_name is moved to the \p new_name.
	     * \param[in] old_name path of the file or directory to be renamed.
	     * \param[in] new_name new path of the file or directory.
	     * \retval true if the file was renamed successfully.
	     * \retval false otherwise.
	     */
	    virtual bool rename_file(
		const std::string& old_name, const std::string& new_name
	    );

	    /**
	     * \brief Gets a file last modification time.
	     * \param[in] path the path to an existing file or directory.
	     * \return the last modification time in seconds
	     */
	    virtual Numeric::uint64 get_time_stamp(const std::string& path);

	    /**
	     * \brief Marks a filename as executable.
	     * \details On unix, it chmods the file, on Windows, does nothing.
	     * \param[in] filename name of the file to be made executable
	     * \retval true on success.
	     * \retval false otherwise.
	     */
	    virtual bool set_executable_flag(const std::string& filename);


	    /**
	     * \brief Modifies the last modification time of a file.
	     * \param[in] filename name of the file.
	     * \retval true on success.
	     * \retval false otherwise.
	     */
	    virtual bool touch(const std::string& filename);

	    /**
	     * \brief Normalizes a path.
	     * \details A path is normalized if it is absolute and it does not 
	     *  contain any "../" component.
	     * \param[in] path the path to be normalized. The path can have 
	     *  components that do not exist.
	     * \return the normalized path
	     */
	    virtual std::string normalized_path(const std::string& path);


	    /**
	     * \brief Gets the current user's home directory.
	     * \return The path to the current user's home directory 
	     *  as a string.
	     */
	    virtual std::string home_directory();

	    /**
	     * \brief Gets the current user's home directory.
	     * \details Under unix, it returns the content of the HOME 
	     *  environment
	     *  variable. Under Windows, it returns the "My Documents" 
	     *  directory.
	     * \return The path to the current user's home directory 
	     *  as a string.
	     */
	    virtual std::string documents_directory();


	    /**
	     * \brief Load file contents in a string.
	     * \param[in] path the path to the file
	     * \return a string with the contents of the file.
	     */
	    virtual std::string load_file_as_string(const std::string& path);
	    
	    /************************ OS-independent **************************/
	    
	    /**
	     * \brief Gets a path extension
	     * \details Extracts the extension from the path \p path, 
	     * that is any character that appear after the last dot (.) 
	     * and after any
	     * directory separator character. If \p path has no extension, the
	     * empty string is returned.
	     *
	     * Examples
	     * - extension("/dir/file.cpp") -> "cpp"
	     * - extension("file") -> ""
	     * - extension("/dir.ext/file") -> ""
	     *
	     * \param[in] path the path to a file or directory
	     * \return the path's extension (without the dot), or the empty
	     * string if none.
	     */
	    virtual std::string extension(const std::string& path);

	    /**
	     * \brief Gets a path base name
	     * \details Extracts the base name from the path \p path, 
	     * that is any
	     * character that appear after the last directory separator. If
	     * parameter \p remove_extension is \c true (the default), the
	     * extension is removed from the base name, otherwise is it kept. If
	     * the path does not contain any directory separator, the whole path
	     * is returned.
	     *
	     * Examples
	     * - base_name("/dir/file.cpp") -> "file"
	     * - base_name("/dir/file.cpp", false) -> "file.cpp"
	     * - base_name("file") -> "file"
	     *
	     * \param[in] path the path to a file or directory
	     * \param[in] remove_extension whether to remove the extension from
	     * the base name or not.
	     */
	    virtual std::string base_name(
		const std::string& path, bool remove_extension = true
	    );

	    /**
	     * \brief Gets a path directory
	     * \details Extracts the directory from the path \p path, 
	     *  that is any character that appear before the last directory 
	     *  separator. If the path does not contain any directory 
	     *  separator, string "." is returned.
	     *
	     * Examples
	     * - dir_name("/dir/file.cpp") -> "dir"
	     * - dir_name("file") -> "."
	     * - dir_name("/") -> "/"
	     *
	     * \param[in] path the path to a file or directory
	     * \return the path directory or "." if none
	     */
	    virtual std::string dir_name(const std::string& path);

	    /**
	     * \brief Lists directory contents
	     * \details Lists all the files and sub-directories in the directory
	     * specified by \p path, and stores the list in \p result. Special
	     * entries "." and ".." are not stored in \p result. If parameter
	     * recursive is set to \c true, \p result will include the entries 
	     * of all sub-directories in \p path recursively.
	     * \param[in] path the path to an existing directory
	     * \param[in] result output vector of entries in \p path
	     * \param[in] recursive recursively traverses all sub-directories in
	     * \p path
	     */
	    virtual void get_directory_entries(
		const std::string& path,
		std::vector<std::string>& result, bool recursive
	    );

	    /**
	     * \brief Lists files in a directory
	     * \details Lists all the files in the directory specified by 
	     *  \p path, and stores the list in \p result. Special entries "." 
	     *  and ".." are not stored in \p result. If parameter recursive 
	     *  is set to \c true, \p result will include the entries of all 
	     *  sub-directories in \p path recursively.
	     * \param[in] path the path to an existing directory
	     * \param[in] result output vector of files in \p path
	     * \param[in] recursive recursively traverses all sub-directories in
	     * \p path
	     * \see get_directory_entries()
	     */
	    virtual void get_files(
		const std::string& path,
		std::vector<std::string>& result, bool recursive = false
	    );

	    /**
	     * \brief Lists sub-directories in a directory
	     * \details Lists all the sub-directories in the directory specified
	     * by \p path, and stores the list in \p result. Special entries "."
	     * and ".." are not stored in \p result. If parameter recursive 
	     * is set to \c true, \p result will include the entries of all
	     * sub-directories in \p path recursively.
	     * \param[in] path the path to an existing directory
	     * \param[in] result output vector of sub-directories in \p path
	     * \param[in] recursive recursively traverses all sub-directories in
	     * \p path
	     * \see get_directory_entries()
	     */
	    virtual void get_subdirectories(
		const std::string& path,
		std::vector<std::string>& result, bool recursive = false
	    );

	    /**
	     * \brief Converts a path to Unix format
	     * \details It changes all Windows "\" directory separators into 
	     *  Unix "/" directory separators.
	     * \param[in,out] path the path to be converted
	     */
	    virtual void flip_slashes(std::string& path);

	    /**
	     * \brief Copies a file
	     * \param[in] from name of the file to be copied
	     * \param[out] to name of the copy
	     * \retval true if the copy was successful
	     * \retval false otherwise
	     */
	    virtual bool copy_file(
		const std::string& from, const std::string& to
	    );
	};

	/**
	 * \brief Implementation of a file system stored in memory.
	 */
	class GEOGRAM_API MemoryNode : public Node {
	public:

	    /**
	     * \brief MemoryNode constructor.
	     * \param[in] path full path to this node.
	     */
	    MemoryNode(const std::string& path="/") : path_(path) {
	    }
	    
	    /** \copydoc Node::copy_file() */
	    bool copy_file(
		const std::string& from, const std::string& to
	    ) override ;

	    /** \copydoc Node::load_file_as_string() */	    
	    std::string load_file_as_string(const std::string& path) override;

	    /** \copydoc Node::is_file() */
	    virtual bool is_file(const std::string& path) override;

	    /** \copydoc Node::is_directory() */	    
	    virtual bool is_directory(const std::string& path) override;

	    /** \copydoc Node::create_directory() */	    	    
	    virtual bool create_directory(const std::string& path) override;

	    /** \copydoc Node::delete_directory() */	    	    	    
	    virtual bool delete_directory(const std::string& path) override;

	    /** \copydoc Node::delete_file() */
	    virtual bool delete_file(const std::string& path) override;

	    /** \copydoc Node::get_directory_entries() */
	    bool get_directory_entries(
		const std::string& path, std::vector<std::string>& result
	    ) override;


	    /** \copydoc Node::rename_file() */
	    bool rename_file(
		const std::string& old_name, const std::string& new_name
	    ) override;

	    /**
	     * \brief Gets the contents of a file.
	     * \param[in] path the path to the file.
	     * \return a const pointer to the contents of the file.
	     */
	    const char* get_file_contents(const std::string& path);

	    /**
	     * \brief Creates a file.
	     * \param[in] path the path to the file
	     * \param[in] content a const pointer to the contents of the file
	     */
	    bool create_file(const std::string& path, const char* content);
	    
	protected:
	    /**
	     * \brief Splits a path.
	     * \param[in] path the path
	     * \param[out] leadingsubdir the leading subdirectory or the
	     *  empty string
	     * \param[out] rest the rest of the path
	     */
	    static void split_path(
		const std::string& path, std::string& leadingsubdir, 
		std::string& rest
	    );
	    
	private:
	    std::string path_;
	    std::map<std::string, SmartPointer<MemoryNode> > subnodes_;
	    std::map<std::string, const char*> files_;
	};
	
	typedef SmartPointer<Node> Node_var;

	/**********************************************************/

        /**
         * \brief Initializes the FileSystem library.
         * \details This function is automatically called during
         *  Geogram startup. It should not be called by client
         *  code.
         */
	void GEOGRAM_API initialize();

        /**
         * \brief Terminates the FileSystem library.
         * \details This function is automatically called during
         *  Geogram shutdown. It should not be called by client
         *  code.
         */
	void GEOGRAM_API terminate();
	
        /** \copydoc FileSystem::Node::is_file() */
        bool GEOGRAM_API is_file(const std::string& path);

        /** \copydoc FileSystem::Node::is_directory() */
        bool GEOGRAM_API is_directory(const std::string& path);

	/** \copydoc FileSystem::Node::create_directory() */
        bool GEOGRAM_API create_directory(const std::string& path);

	/** \copydoc FileSystem::Node::delete_directory() */	
        bool GEOGRAM_API delete_directory(const std::string& path);

	/** \copydoc FileSystem::Node::delete_file() */	
        bool GEOGRAM_API delete_file(const std::string& path);

	/** \copydoc FileSystem::Node::get_directory_entries() */	
        bool GEOGRAM_API get_directory_entries(
            const std::string& path, std::vector<std::string>& result
        );

	/** \copydoc FileSystem::Node::get_current_working_directory() */	
        std::string GEOGRAM_API get_current_working_directory();
        bool GEOGRAM_API set_current_working_directory(
            const std::string& path
        );

	/** \copydoc FileSystem::Node::rename_file() */	
        bool GEOGRAM_API rename_file(
            const std::string& old_name, const std::string& new_name
        );

	/** \copydoc FileSystem::Node::get_time_stamp() */	
        Numeric::uint64 GEOGRAM_API get_time_stamp(
            const std::string& path
        );

	/** \copydoc FileSystem::Node::extension() */	
        std::string GEOGRAM_API extension(const std::string& path);

	/** \copydoc FileSystem::Node::base_name() */	
        std::string GEOGRAM_API base_name(
            const std::string& path, bool remove_extension = true
        );
	
	/** \copydoc FileSystem::Node::dir_name() */	
        std::string GEOGRAM_API dir_name(const std::string& path);

	/** \copydoc FileSystem::Node::get_directory_entries() */	
        void GEOGRAM_API get_directory_entries(
            const std::string& path,
            std::vector<std::string>& result, bool recursive
        );

	/** \copydoc FileSystem::Node::get_files() */	
        void GEOGRAM_API get_files(
            const std::string& path,
            std::vector<std::string>& result, bool recursive = false
        );

	/** \copydoc FileSystem::Node::get_subdirectories() */	
        void GEOGRAM_API get_subdirectories(
            const std::string& path,
            std::vector<std::string>& result, bool recursive = false
        );

	/** \copydoc FileSystem::Node::flip_slashes() */	
        void GEOGRAM_API flip_slashes(std::string& path);

	/** \copydoc FileSystem::Node::copy_file() */	
        bool GEOGRAM_API copy_file(
            const std::string& from, const std::string& to
        );

	/** \copydoc FileSystem::Node::set_executable_flag() */	
        bool GEOGRAM_API set_executable_flag(const std::string& filename);

	/** \copydoc FileSystem::Node::touch() */	
        bool GEOGRAM_API touch(const std::string& filename);

	/** \copydoc FileSystem::Node::normalized_path() */	
        std::string GEOGRAM_API normalized_path(const std::string& path);

	/** \copydoc FileSystem::Node::home_directory() */	
        std::string GEOGRAM_API home_directory();

	/** \copydoc FileSystem::Node::documents_directory() */	
        std::string GEOGRAM_API documents_directory();

	/**
	 * \brief Gets the root of the file system.
	 * \param[out] root a pointer to the root of 
	 *  the FileSystem.
	 */
	void GEOGRAM_API get_root(Node*& root);
	
#ifdef GEO_OS_EMSCRIPTEN
	/**
	 * \brief Declares a function to be called whenever the file system
	 *  changes.
	 * \details The function will be called when the user loads a file
	 *  using the button in the webpage.
	 * \param[in] callback the function to be called.
	 */
	void set_file_system_changed_callback(void(*callback)());
#endif	
	
    }
}

#endif

