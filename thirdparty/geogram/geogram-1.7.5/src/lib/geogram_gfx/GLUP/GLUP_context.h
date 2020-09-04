/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef GEOGRAM_GFX_GLUP_GLUP_CONTEXT
#define GEOGRAM_GFX_GLUP_GLUP_CONTEXT

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/GLUP/GLUP_marching_cells.h>
#include <geogram_gfx/basic/GLSL.h>
#include <map>

/**
 * \file geogram_gfx/GLUP/GLUP_context.h
 * \brief Internal implementation of GLUP context.
 */

#ifdef GEO_GL_NO_DOUBLES
typedef double GLdouble;
#endif

namespace GLUP {
    using namespace GEO;

    /**
     * \brief Computes the inverse of a 4x4 matrix.
     * \param[out] inv the computed inverse of \p m
     * \param[in] m pointer to the input matrix
     * \retval GL_TRUE if the matrix \p m is invertible
     * \retval GL_FALSE if the matrix \p m is singular. Then
     *   \p inv receives the transpose of the comatrix of \p m
     */
    GLboolean invert_matrix(GLfloat inv[16], const GLfloat m[16]);

    /**
     * \brief Computes the inverse of a 4x4 matrix.
     * \param[out] inv the computed inverse of \p m
     * \param[in] m pointer to the input matrix
     * \retval GL_TRUE if the matrix \p m is invertible
     * \retval GL_FALSE if the matrix \p m is singular. Then
     *   \p inv receives the transpose of the comatrix of \p m
     */
    GLboolean invert_matrix(GLdouble inv[16], const GLdouble m[16]);
    
    /**
     * \brief Computes the product of two 4x4 matrices
     * \param[out] out the computed product \p m1 * \p m2
     * \param[in] m1 , m2 pointers to the input matrices
     */
    void mult_matrices(
        GLfloat out[16], const GLfloat m1[16], const GLfloat m2[16]
    );

    /**
     * \brief Computes the product of two 4x4 matrices
     * \param[out] out the computed product \p m1 * \p m2
     * \param[in] m1 , m2 pointers to the input matrices
     */
    void mult_matrices(
        GLdouble out[16], const GLdouble m1[16], const GLdouble m2[16]
    );

    /**
     * \brief Computes the product of a 4x4 matrix and a vector.
     * \param[out] out the computed product \p m * \p v
     * \param[in] m pointer to the input matrix
     * \param[in] v pointer to the input vector
     * \TODO: it seems that in GLSL, w = M*v uses the transpose form,
     *   maybe I should change the names !!
     */
    void mult_matrix_vector(
        GLfloat out[4], const GLfloat m[16], const GLfloat v[4]
    );


    /**
     * \brief Computes the product of the transpose of a 
     *   4x4 matrix and a vector.
     * \param[out] out the computed product \p m * \p v
     * \param[in] m pointer to the input matrix
     * \param[in] v pointer to the input vector
     * \TODO: it seems that in GLSL, w = M*v uses this one, maybe
     *  I should change the names !!
     */
    void mult_transpose_matrix_vector(
        GLfloat out[4], const GLfloat m[16], const GLfloat v[4]
    );


    /**
     * \brief Computes the product of the transpose of a 
     *   4x4 matrix and a vector.
     * \param[out] out the computed product \p m * \p v
     * \param[in] m pointer to the input matrix
     * \param[in] v pointer to the input vector
     * \TODO: it seems that in GLSL, w = M*v uses this one, maybe
     *  I should change the names !!
     */
    void mult_transpose_matrix_vector(
        GLdouble out[4], const GLdouble m[16], const GLdouble v[4]
    );
    
    /**
     * \brief Computes the product of a 4x4 matrix and a vector.
     * \param[out] out the computed product \p m * \p v
     * \param[in] m pointer to the input matrix
     * \param[in] v pointer to the input vector
     */
    void mult_matrix_vector(
        GLdouble out[4], const GLdouble m[16], const GLdouble v[4] 
    );

    /**
     * \brief Transposes a matrix in-place.
     * \param[in,out] m a pointer to the 16 single-precision
     *  floating point coefficients of the matrix to be transposed.
     */
    void transpose_matrix(GLfloat m[16]);

    /**
     * \brief Transposes a matrix in-place.
     * \param[in,out] m a pointer to the 16 double-precision
     *  floating point coefficients of the matrix to be transposed.
     */
    void transpose_matrix(GLdouble m[16]);
    
    /**
     * \brief For debugging, outputs a matrix to the standard error.
     * \param[in] m the matrix to be displayed.
     */
    void show_matrix(const GLfloat m[16]);

    /**
     * \brief For debugging, outputs a vector to the standard error.
     * \param[in] v the vector to be displayed
     */
    void show_vector(const GLfloat v[4]);
    
    /**
     * \brief Resets a matrix to the identity matrix.
     * \param[out] out the matrix to be reset.
     */
    void load_identity_matrix(GLfloat out[16]);

    /**
     * \brief Copies a vector of floats.
     * \param[out] to a pointer to the destination vector
     * \param[in] from a const pointer to the source vector
     * \param[in] dim the number of components to copy
     */
    inline void copy_vector(GLfloat* to, const GLfloat* from, index_t dim) {
        Memory::copy(to, from, sizeof(GLfloat)*dim);
    }

    /**
     * \brief Copies a vector of doubles to a vector of floats.
     * \param[out] to a pointer to the destination vector
     * \param[in] from a const pointer to the source vector
     * \param[in] dim the number of components to copy
     */
    inline void copy_vector(GLfloat* to, const GLdouble* from, index_t dim) {
        for(index_t i=0; i<dim; ++i) {
            to[i] = GLfloat(from[i]);
        }
    }

    /**
     * \brief Copies a vector of floats to a vector of doubles.
     * \param[out] to a pointer to the destination vector
     * \param[in] from a const pointer to the source vector
     * \param[in] dim the number of components to copy
     */
    inline void copy_vector(GLdouble* to, const GLfloat* from, index_t dim) {
        for(index_t i=0; i<dim; ++i) {
            to[i] = GLdouble(from[i]);
        }
    }
    
    /**
     * \brief Normalizes a vector.
     * \param[in,out] v a pointer to the 3 coordinates of the 3d 
     *  vector to be normalized.
     */
    inline void normalize_vector(GLfloat v[3]) {
        GLfloat s = 1.0f / ::sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
        v[0] *= s;
        v[1] *= s;
        v[2] *= s;            
    }


    /**
     * \brief Gives the number of vertices for each GLUP primitive.
     * \details The array is indexed by the GLUP primitive type
     *  (one of GLUP_POINTS, GLUP_LINES, ...)
     */
    extern index_t nb_vertices_per_primitive[];
    
    /**********************************************************************/
    
    class Context;
    
    /**
     * \brief A Matrix stack.
     * \details There are three matrix stacks in a context,
     *  for modelview matrices, projection matrices and
     *  texture coordinates.
     */
    class MatrixStack {
    public:

        /**
         * \brief Maximum number of matrices in a stack.
         */
        static const int MAX_DEPTH=16;

        /**
         * \brief MatrixStack constructor.
         */
        MatrixStack() : top_(0) {
            load_identity_matrix(top());
        }

        /**
         * \brief Gets the matrix on the top of
         *  the stack.
         * \return a pointer to the coefficients of
         *  the matrix.
         */
        GLfloat* top() {
            return stack_[top_].data();
        }

        /**
         * \brief Pushes a copy of the top matrix.
         */
        void push() {
            geo_assert(top_ != MAX_DEPTH-1);
            GLfloat* from = top();
            ++top_;
            GLfloat* to = top();
            copy_vector(to, from, 16);
        }

        /**
         * \brief Removes a matrix from the top of
         *  the stack.
         */
        void pop() {
            geo_assert(top_ != 0);
            --top_;
        }

    protected:
        struct Matrix {
            GLfloat coeff[16];
            GLfloat* data() {
                return &coeff[0];
            }
        };
        
    private:
        Matrix stack_[MAX_DEPTH];
        index_t top_;
    };
        

    /**
     * \brief Number of vertices/colors/tex_coords in a GLUP buffer used
     *  by immediate mode.
     * \details Chosen in such a way that indices in there can be stored
     *  in two bytes.
     */
    static const index_t IMMEDIATE_BUFFER_SIZE = 65536;

    /**
     * \brief Index of an ImmediateBuffer in the ImmediateState.
     * \details GLUP_VERTEX_ID_ATTRIBUTE is used internally
     */
    enum GLUPattribute {
        GLUP_VERTEX_ATTRIBUTE    = 0,
        GLUP_COLOR_ATTRIBUTE     = 1,
        GLUP_TEX_COORD_ATTRIBUTE = 2,
	GLUP_NORMAL_ATTRIBUTE    = 3,
	GLUP_VERTEX_ID_ATTRIBUTE = 4
    };

    /**
     * \brief A buffer used by GLUP in immediate mode.
     */
    class ImmediateBuffer {
        
    public:

        /**
         * ImmediateBuffer constructor.
         */
        ImmediateBuffer() :
            data_(nullptr),
            dimension_(0),
            is_enabled_(false),
            VBO_(0) {
        }

        /**
         * ImmediateBuffer destructor.
         */
        ~ImmediateBuffer() {
            delete[] data_;
            if(VBO_ != 0) {
                glDeleteBuffers(1, &VBO_);
                VBO_ = 0;
            }
        }

        void initialize(index_t dim) {
            data_ = new GLfloat[dim * IMMEDIATE_BUFFER_SIZE];
            dimension_ = dim;
            is_enabled_ = true; 
        }

        /**
         * \brief Enables this ImmediateBuffer.
         */
        void enable() {
            is_enabled_ = true;
        }

        /**
         * \brief Disables this ImmediateBuffer.
         */
        void disable() {
            is_enabled_ = false;
        }

        /**
         * \brief Tests whether this ImmediateBuffer is enabled.
         * \retval true if this ImmediateBuffer is enabled
         * \retval false otherwise
         */
        bool is_enabled() const {
            return is_enabled_;
        }
        
        /**
         * \brief Sets the current attribute value.
         * \param[in] x , y , z , w the component of the current attribute
         * \details Components past the dimension of the attribute
         *   are ignored (e.g., if dimension is 2, z and w are ignored).
         */
        void set_current(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
            current_[0] = x;
            current_[1] = y;
            current_[2] = z;
            current_[3] = w;
        }

        /**
         * \brief Copies the current attribute value to a specified
         *  vertex in this buffer.
         * \param[in] v the vertex index
         * \pre v < IMMEDIATE_BUFFER_SIZE
         */
        void copy_current_to(index_t v) {
            geo_debug_assert(v < IMMEDIATE_BUFFER_SIZE);
            if(is_enabled()) {
                copy_vector(element_ptr(v), current_, dimension());
            }
        }

        /**
         * \brief Copies this attribute from a vertex to another one.
         * \param[in] to index of the destination vertex
         * \param[in] from index of the source vertex
         * \pre to < IMMEDIATE_BUFFER_SIZE && from < IMMEDIATE_BUFFER_SIZE
         */
        void copy(index_t to, index_t from) {
            geo_debug_assert(to < IMMEDIATE_BUFFER_SIZE);
            geo_debug_assert(from < IMMEDIATE_BUFFER_SIZE);
            if(is_enabled()) {
                copy_vector(element_ptr(to), element_ptr(from), dimension());
            }
        }

        /**
         * \brief Gets the dimension of the attribute.
         * \return the number of components of the attribute
         */
        index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Gets the size of the memory used by the buffer.
         * \return the size of the buffer in bytes.
         */
        size_t size_in_bytes() const {
            return IMMEDIATE_BUFFER_SIZE * dimension() * sizeof(GLfloat);
        }
        
        /**
         * \brief Gets a pointer to one attribute value by index.
         * \param[in] v index of the vertex
         * \return a pointer to the attribute, i.e. an array of 
         *  \p dimension() GLfloats
         */
        GLfloat* element_ptr(index_t v) {
            geo_debug_assert(v < IMMEDIATE_BUFFER_SIZE);
            return data_ + v*dimension_;
        }

        /**
         * \brief Gets a pointer to the data.
         * \return a pointer to the first attribute. All the storage
         *  is contiguous in memory.
         */
        GLfloat* data() {
            return data_;
        }

        /**
         * \brief Gets the Vertex Buffer Object.
         * \return a modifiable reference to the Id of the Vertex Buffer
         *  Object. Can be zero if no VBO is used.
         */
        GLuint& VBO() {
            return VBO_;
        }

        /**
         * \brief ImmediateBuffer copy constructor.
         * \param[in] rhs the ImmediateBuffer to be copied
         * \details Should be only called with uninitialized ImmediateBuffer
         *  (else triggers an assertion failure).
         */
        ImmediateBuffer(
            const ImmediateBuffer& rhs
        ) {
            data_ = rhs.data_;
            dimension_ = rhs.dimension_;
            is_enabled_ = rhs.is_enabled_;
            VBO_ = rhs.VBO_;
            current_[0] = rhs.current_[0];
            current_[1] = rhs.current_[1];
            current_[2] = rhs.current_[2];
            current_[3] = rhs.current_[3];
            geo_assert(data_ == nullptr);
        }
        
    private:
        GLfloat* data_;
        GLfloat current_[4];
        index_t dimension_; 
        bool is_enabled_;
        GLuint VBO_;
    };

    /**
     * \brief Stores all the buffers used to implement
     *  the immediate-mode interface.
     */
    class ImmediateState {
    public:
        /**
         * \brief ImmediateState constructor.
         */
        ImmediateState() :
            current_vertex_(0),
            max_current_vertex_(0),
            primitive_(GLUP_POINTS),
            VAO_(0)
	{
            buffer[GLUP_VERTEX_ATTRIBUTE].initialize(4);
            buffer[GLUP_COLOR_ATTRIBUTE].initialize(4);
            buffer[GLUP_TEX_COORD_ATTRIBUTE].initialize(4);
            buffer[GLUP_NORMAL_ATTRIBUTE].initialize(4);	    
            
            // Vertex is always enabled
            buffer[GLUP_VERTEX_ATTRIBUTE].enable();
        }

        /**
         * \brief ImmediateState destructor.
         */
        ~ImmediateState() {
            if(VAO_ != 0) {
                glupDeleteVertexArrays(1, &VAO_);
                VAO_ = 0;
            }
        }
        
        /**
         * \brief Gets the Vertex Array Object.
         * \return a modifiable reference to the Id of the Vertex Array Object.
         *   Can be 0 if no VAO is used.
         */
        GLuint& VAO() {
            return VAO_;
        }
        
        /**
         * \brief Copies an element, i.e. all the attributes
         *  attached to a vertex.
         * \param[in] to index of the destination vertex
         * \param[in] from index of the source vertex
         * \details Only attributes that are enabled are copied.
         */
        void copy_element(index_t to, index_t from) {
            for(index_t i=0; i<NB_IMMEDIATE_BUFFERS; ++i) {
                buffer[i].copy(to, from);
            }
        }

        
        /**
         * \brief Configures the immediate state for rendering
         *  primitives of a given type.
         * \param[in] primitive type of the primitives to be rendered
         */
        void begin(GLUPprimitive primitive) {
            current_vertex_ = 0;
            max_current_vertex_ =
            IMMEDIATE_BUFFER_SIZE - (
                IMMEDIATE_BUFFER_SIZE %
                nb_vertices_per_primitive[primitive]
            );
            primitive_ = primitive;
        }

        /**
         * \brief Advances to the next vertex.
         * \details This copies all the current values of all enabled attributes
         *  to the current vertex position.
         */
        void next_vertex() {
            for(index_t i=0; i<NB_IMMEDIATE_BUFFERS; ++i) {
                buffer[i].copy_current_to(current_vertex_);
            }
            ++current_vertex_;
        }

        /**
         * \brief Tests whether the buffers are full.
         * \details When buffers are fulled, their contents need to be sent
         *  to OpenGL before calling reset(). These operations are done
         *  by the Context.
         */
        bool buffers_are_full() {
            return (current_vertex_ == max_current_vertex_);
        }

        /**
         * \brief Resets the current vertex index to zero.
         */
        void reset() {
            current_vertex_ = 0;
        }

        /**
         * \brief Gets the primitive currently rendered, i.e.
         *  the argument to the latest invocation of begin()
         */
        GLUPprimitive primitive() const {
            return primitive_;
        }

        /**
         * \brief Gets the number of vertices stored in the buffers.
         */
        index_t nb_vertices() const {
            return current_vertex_;
        }

        /**
         * \brief Gets the number of primitives stored in the buffers.
         */
        index_t nb_primitives() const {
            return current_vertex_ / nb_vertices_per_primitive[
                primitive_
            ];
        }

	/**
	 * \brief Gets the maximum number of vertices in the buffer
	 *  before the buffer is flushed.
	 * \details This number depends on the number of vertices per
	 *  primitive.
	 */
	index_t max_current_vertex() const {
	    return max_current_vertex_;
	}

	/**
	 * \brief Sets the current vertex.
	 * \details This defines the number of stored vertices in this
	 *  buffer.
	 * \param[in] v the index of the current vertex.
	 */
	void set_current_vertex(index_t v) {
	    geo_debug_assert(v <= max_current_vertex_);
	    current_vertex_ = v;
	}

	enum { NB_IMMEDIATE_BUFFERS = 4 };
        ImmediateBuffer buffer[NB_IMMEDIATE_BUFFERS];
        
    private:
        index_t current_vertex_;
        index_t max_current_vertex_;
        GLUPprimitive primitive_;
        GLuint VAO_;
    };
    

    /**********************************************************/
    
    /**
     * \brief Base class for representing GLUP state variables.
     */
    class StateVariableBase {
    public:

        /**
         * \brief StateVariableBase default constructor.
         */
        StateVariableBase() : address_(nullptr), context_(nullptr) {
        }

        /**
         * \brief StateVariableBase constructor.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically).
         */
        StateVariableBase(
            Context* context, const char* name
        ) {
            initialize(context,name);
        }

        /**
         * \brief Initializes a StateVariableBase.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically
         *  when searching for the variable in the state).
         */
        void initialize(Context* context, const char* name);

        /**
         * \brief Gets the name of this StateVariableBase.
         * \return a const reference to the name, without
         *  "GLUPStateBlock." prepended to it.
         */
        const std::string& name() const {
            return name_;
        }
        
    protected:
        friend class Context;
        
        /**
         * \brief Gets the address of the StateVariableBase.
         * \return a pointer to the variable in the client-side
         *  representation of the UBO.
         */
        Memory::pointer address() const {
            return address_;
        }
        
        /**
         * \brief Indicates that the variables in the context
         *  need to be sent to OpenGL.
         */
        void flag_uniform_buffer_as_dirty();

        Memory::pointer address_;
        Context* context_;
        std::string name_;
    };

    /**
     * \brief A GLUP state variable of a given type.
     * \tparam T the type of the state variable
     */
    template <class T> class StateVariable : public StateVariableBase {
    public:

        /**
         * \brief StateVariable default constructor.
         */
        StateVariable() {
        }

        /**
         * \brief StateVariableBase constructor.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically).
         * \param[in] value initial value of the variable
         */
        StateVariable(
            Context* context, const char* name, T value
        ) : StateVariableBase(context, name) {
            set(value);
        }

        /**
         * \brief Initializes a StateVariable.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically).
         * \param[in] value initial value of the variable
         */
        void initialize(Context* context, const char* name, T value) {
            StateVariableBase::initialize(context, name);
            set(value);
        }

        /**
         * \brief Gets the value.
         * \return the value of this StateVariable.
         */
        T get() const {
            return *reinterpret_cast<T*>(address_);
        }

        /**
         * \brief Sets the value.
         * \param[in] val the new value
         * \details flags the uniform buffer as dirty
         */
        void set(T val) {
            *reinterpret_cast<T*>(address_) = val;
            flag_uniform_buffer_as_dirty();
        }
    };

    /**
     * \brief A GLUP state variable that contains an array
     *  of floating points. This concerns both vectors and
     *  matrices.
     */
    class FloatsArrayStateVariable : public StateVariableBase {
    public:

        /**
         * \brief FloatsArrayStateVariable default constructor.
         */
        FloatsArrayStateVariable() {
        }

        /**
         * \brief FloatsArrayStateVariable constructor.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically).
         */
        FloatsArrayStateVariable(
            Context* context, const char* name
        ) : StateVariableBase(context, name) {
        }

        /**
         * \brief Gets a pointer to the variable.
         * \return a const pointer to the first GLUPfloat stored
         *  in the variable
         */
        const GLUPfloat* get_pointer() const {
            return reinterpret_cast<const GLUPfloat*>(address_);
        }

        /**
         * \brief Gets a modifiable pointer to the variable.
         * \return a modifiable pointer to the first GLUPfloat stored
         *  in the variable
         * \details This flags the uniform buffer as dirty in the
         *  Context.
         */
        GLUPfloat* get_pointer() {
            // This is a non-const pointer, therefore it will be
            // probably modified by client code (else the 'const'
            // version of get_pointer() would have been called).
            flag_uniform_buffer_as_dirty();
            return reinterpret_cast<GLUPfloat*>(address_);
        }
    };

    /**
     * \brief A GLUP state variable that contains a vector. 
     * \details This corresponds to vec2, vec3, vec4 GLSL types.
     */
    class VectorStateVariable : public FloatsArrayStateVariable {
    public:

        /**
         * \brief VectorStateVariable default constructor.
         */
        VectorStateVariable() : dimension_(0) {
        }

        /**
         * \brief VectorStateVariable constructor.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically)
         * \param[in] dimension 2 for vec2, 3 for vec3, 4 for vec4
         */
        VectorStateVariable(
            Context* context, const char* name, index_t dimension
        ) : FloatsArrayStateVariable(context, name), dimension_(dimension) {
            clear();
        }

        /**
         * \brief Initializes a VectorStateVariable.
         * \param[in] context a pointer to the GLUP Context
         * \param[in] name the name of the variable, without 
         *  "GLUPStateBlock." (it is prepended automatically)
         * \param[in] dimension 2 for vec2, 3 for vec3, 4 for vec4
         */
        void initialize(Context* context, const char* name, index_t dimension) {
            FloatsArrayStateVariable::initialize(context, name);
            dimension_ = dimension;
            clear();
        }

        /**
         * \brief Gets the dimension.
         * \return the number of components of this vector
         */
        index_t dimension() const {
            return dimension_;
        }
        
        /**
         * \brief Gets the value.
         * \param[out] x a pointer to an array of dimension()
         *  GLfloats, where to store the value
         */
        void get(GLUPfloat* x) const {
            Memory::copy(x, address_, sizeof(GLUPfloat)*dimension_);
        }

        /**
         * \brief Sets the value.
         * \param[in] x a const pointer to an array of dimension()
         *  GLfloats that contains the new value
         */
        void set(const GLUPfloat* x) {
            Memory::copy(address_, x, sizeof(GLUPfloat)*dimension_);
            flag_uniform_buffer_as_dirty();
        }

        /**
         * \brief clears the vector to its default value.
         * \details For vec2, default value is (0.0, 0.0), for
         *  vec3, it is (0.0, 0.0, 0.0) and for vec4 it is 
         *  (0.0, 0.0, 0.0, 1.0)
         */
        void clear() {
            Memory::clear(address_, sizeof(GLUPfloat)*dimension_);
            if(dimension_ == 4) {
                reinterpret_cast<GLUPfloat*>(address_)[3] = 1.0f;
            }
            flag_uniform_buffer_as_dirty();            
        }
            
    protected:
        index_t dimension_;
    };


    /**
     * \brief The set of state variables that represent GLUP uniform state.
     */
    struct UniformState {
        vector< StateVariable<GLboolean> > toggle;
        vector< VectorStateVariable>       color;
        VectorStateVariable                light_vector;
        VectorStateVariable                light_half_vector;
        StateVariable<GLfloat>             point_size;
        StateVariable<GLfloat>             mesh_width;
        StateVariable<GLfloat>             cells_shrink;
        StateVariable<GLint>               picking_mode;
        StateVariable<GLint>               picking_id;      
        StateVariable<GLint>               base_picking_id; 
        StateVariable<GLint>               clipping_mode;
        StateVariable<GLint>               texture_mode;
        StateVariable<GLint>               texture_type;
        StateVariable<GLfloat>             alpha_threshold;
	StateVariable<GLfloat>             specular;
        VectorStateVariable                clip_plane;
        VectorStateVariable                world_clip_plane;
        VectorStateVariable                clip_clip_plane;	
        FloatsArrayStateVariable           modelview_matrix;
        FloatsArrayStateVariable           modelviewprojection_matrix;
        FloatsArrayStateVariable           projection_matrix;        
        FloatsArrayStateVariable           normal_matrix;
        FloatsArrayStateVariable           texture_matrix;
	FloatsArrayStateVariable           inverse_modelviewprojection_matrix;
        FloatsArrayStateVariable           inverse_modelview_matrix;
        FloatsArrayStateVariable           inverse_projection_matrix;		
	VectorStateVariable                viewport;
    };
    
    /**********************************************************************/

    /**
     * \brief Stores the programs and vertex array object used to display
     *  a primitive of a given type.
     */
    struct PrimitiveInfo {

	typedef Numeric::uint64 ShaderKey;
	
        /**
         * \brief PrimitiveInfo constructor.
         */
        PrimitiveInfo():
            GL_primitive(0),
            VAO(0),
            elements_VBO(0),
            nb_elements_per_primitive(0),
            primitive_elements(nullptr),
            vertex_gather_mode(false),
            implemented(false) {
        }

        /**
         * \brief PrimitiveInfo copy constructor.
         * \param[in] rhs the PrimitiveInfo to be copied.
         * \details Should be only called with uninitialized PrimitiveInfo
         *  (else triggers an assertion failure).
         */
         PrimitiveInfo(const PrimitiveInfo& rhs) : shader_map(rhs.shader_map) {
            GL_primitive = rhs.GL_primitive;
            VAO = rhs.VAO;
            elements_VBO = rhs.elements_VBO;
            nb_elements_per_primitive = rhs.nb_elements_per_primitive;
            primitive_elements = rhs.primitive_elements;
            vertex_gather_mode = rhs.vertex_gather_mode;
            implemented = rhs.implemented;
            geo_assert(GL_primitive == 0);
            geo_assert(nb_elements_per_primitive == 0);
        }
        
        /**
         * \brief PrimitiveInfo destructor.
         * \details Deletes the programs and vertex array object if need be.
         */
        ~PrimitiveInfo() {
	    for(auto& it : shader_map) {
		if(it.second != 0) {
		    glDeleteProgram(it.second);
		    it.second = 0;
		}
	    }
            if(elements_VBO != 0) {
                glDeleteBuffers(1, &elements_VBO);
            }
            if(VAO != 0) {
                glupDeleteVertexArrays(1,&VAO);
                VAO = 0;
            }
        }

	bool program_is_initialized(ShaderKey k) const {
	    return (shader_map.find(k) != shader_map.end());
	}

	GLuint program(ShaderKey k) const {
	    auto it = shader_map.find(k);
	    return ((it == shader_map.end()) ? 0 : it->second);
	}
	
        GLenum GL_primitive;
	std::map<ShaderKey, GLuint> shader_map;
        GLuint VAO;
        GLuint elements_VBO;
        index_t nb_elements_per_primitive;
        index_t* primitive_elements;
        bool vertex_gather_mode;
        bool implemented;
    };
    
    /**********************************************************************/
    
    /**
     * \brief GLUP context stores a Uniform Buffer Object with state
     *  variables similar to OpenGL's fixed functionality pipeline, and
     *  a set of Vertex Buffer Objects to emulate OpenGL's immediate mode.
     */
    class Context : public GLSL::PseudoFileProvider {
    public:
        /**
         * \brief Gets the GLSL declaration of GLUP uniform state.
         * \return a pointer to GLSL source code that declares 
         *  GLUP uniform state.
         * \details Can be used by client-code shaders that need to
         *  have access to the GLUP uniform state. This corresponds
         *  to the contents of GLUPGLSL/state.h
         */
        static const char* uniform_state_declaration();

        /**
         * \brief Context constructor.
         */
        Context();

        /**
         * \brief Context destructor.
         */
        virtual ~Context();


        /**
         * \brief Gets the profile name associated with this context.
         */
        virtual const char* profile_name() const = 0; 
        
        /**
         * \brief Tests whether a given GLUP primitive supports array mode.
         * \details If array mode is supported, then one can use glupDrawArray()
         *  and glupDrawElements() with the specified primitive.
         * \param[in] prim the primitive to be tested.
         * \retval true if array mode is supported with \p prim
         * \retval false otherwise
         */
        virtual bool primitive_supports_array_mode(GLUPprimitive prim) const;
        
        /**
         * \brief Creates the uniform state and GLSL programs.
         * \details This function may throw exceptions if GLSL 
         *  functionalities are not implemented in the OpenGL driver.
         */
        virtual void setup();
        
        /**
         * \brief Binds GLUP uniform state to a program.
         * \param[in] program the id of the GLSL program
         * \details If the program uses GLUP, then it 
         *  binds the program to GLUP uniform state, else this
         *  function does nothing.
         */
        virtual void bind_uniform_state(GLuint program);

        /**
         * \brief Replaces the top of the current matrix stack
         *  with the specified matrix.
         * \param[in] m the matrix that will replace the top of
         *  the current matrix stack
         */
        void load_matrix(const GLfloat m[16]) {
            copy_vector(matrix_stack_[matrix_mode_].top(), m, 16);
            flag_matrices_as_dirty();
        }

        /**
         * \brief Replaces the top of the current matrix stack
         *  with the identity matrix.
         */
        void load_identity() {
            load_identity_matrix(matrix_stack_[matrix_mode_].top());
            flag_matrices_as_dirty();            
        }

        /**
         * \brief Post-multiplies the top of the current matrix stack
         *   with the specified matrix.
         * \param[in] m the matrix that will post-multiply the
         *   top of the current matrix stack.
         * \see matrix_mode()
         */
        void mult_matrix(const GLfloat m[16]) {
            GLfloat product[16];
            mult_matrices(product,m,matrix_stack_[matrix_mode_].top());
            load_matrix(product);
        }

        /**
         * \brief Pushes a copy of the top of the current stack matrix
         *  onto the current stack matrix.
         * \see matrix_mode(), pop_matrix()
         */
        void push_matrix() {
            matrix_stack_[matrix_mode_].push();
        }

        /**
         * \brief Pops the top of the current stack matrix.
         */
        void pop_matrix() {
            matrix_stack_[matrix_mode_].pop();
            flag_matrices_as_dirty();
        }
        
        /**
         * \brief Sets the current matrix stack.
         * \param[in] matrix one of GLUP_MODELVIEW, GLUP_PROJECT
         * \details This determines on which matrix stack set_matrix(),
         *  mult_matrix(), push_matrix() and pop_matrix() operate.
         */
        void set_matrix_mode(GLUPmatrix matrix) {
            matrix_mode_ = matrix;
        }

        /**
         * \brief Gets the current matrix stack.
         * \return The current matrix stack, i.e. 
         *  one of GLUP_MODELVIEW, GLUP_PROJECT
         */
        GLUPmatrix get_matrix_mode() const {
            return matrix_mode_;
        }

        /**
         * \brief Creates a new vertex in the immediate mode
         *  buffers.
         * \param[in] x , y , z , w the coordinates of the vertex
         * \details The color and texture coordinates of the new
         *  vertex are initialized from the current color and 
         *  current texture coordinates.
         */
        void immediate_vertex(
            GLfloat x, GLfloat y, GLfloat z=0.0f, GLfloat w=1.0f
        ) {
            immediate_state_.buffer[GLUP_VERTEX_ATTRIBUTE].set_current(x,y,z,w);
            immediate_state_.next_vertex();
            if(immediate_state_.buffers_are_full()) {
                flush_immediate_buffers();
            }
        }

        /**
         * \brief Specifies the current color for the immediate
         *  mode buffers.
         * \param[in] r , g , b , a the components of the current color.
         */
        void immediate_color(
            GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f
        ) {
            immediate_state_.buffer[GLUP_COLOR_ATTRIBUTE].set_current(r,g,b,a);
        }

        /**
         * \brief Specifies the current texture coordinates for the
         *  immediate mode buffers.
         * \param[in] s , t , u , v the current texture coordinates.
         */
        void immediate_tex_coord(
            GLfloat s, GLfloat t=0.0f, GLfloat u=0.0f, GLfloat v=1.0f
        ) {
            immediate_state_.buffer[GLUP_TEX_COORD_ATTRIBUTE].set_current(
                s,t,u,v
            );
        }

        /**
         * \brief Specifies the current normal vector for the
         *  immediate mode buffers.
         * \param[in] x , y , z the current normal vector coordinates.
         */
        void immediate_normal(GLfloat x, GLfloat y, GLfloat z) {
            immediate_state_.buffer[GLUP_NORMAL_ATTRIBUTE].set_current(
                x,y,z,0.0f
            );
        }
	
        /**
         * \brief Sets the user program, to be used instead of
         *  the default GLUP programs for drawing the primitives.
         */
        void set_user_program(GLuint program) {
            user_program_ = program;
        }
        
        /**
         * \brief Begins rendering in immediate mode.
         * \param[in] primitive the primitive to be rendered.
         * \see immediate_vertex(), immediate_color(), immediate_tex_coord()
         */
        virtual void begin(GLUPprimitive primitive);

        /**
         * \brief Ends rendering in immediate mode.
         * \see begin()
         */
        virtual void end();

        /**
         * \brief Draws primitives using current OpenGL array bindings.
         * \details This function operates just like glDrawArrays(), 
         *  except that its \p primitive argument is a GLUPprimitive
         *  instead of regular OpenGL primitive. Internally it uses
         *  a (possibly different) OpenGL primitive, as well as
         *  a GLSL program to reinterpret it.
         * \param[in] primitive the GLUP primitive type
         * \param[in] first first index to be rendered
         * \param[in] count number of vertices to be rendered
         */
        virtual void draw_arrays(
            GLUPprimitive primitive, GLUPint first, GLUPsizei count
        );

        /**
         * \brief Draws primitives using current OpenGL array bindings.
         * \details This function operates just like glDrawElements(), 
         *  except that its \p primitive argument is a GLUPprimitive
         *  instead of regular OpenGL primitive. Internally it uses
         *  a (possibly different) OpenGL primitive, as well as
         *  a GLSL program to reinterpret it.
         * \param[in] primitive the GLUP primitive type
         * \param[in] count number of vertices to be rendered
         * \param[in] type type of element indices, as one of 
         *   GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT
         * \param[in] indices a pointer to where the indices are stored.
         */
        virtual void draw_elements(
            GLUPprimitive primitive, GLUPsizei count,
            GLUPenum type, const GLUPvoid* indices
        );

        /**
         * \brief Gets a pointer to the representation of a uniform
         *  state variable in host memory from its (unqualified) name.
         * \param[in] name the name of the variable, without the suffix
         *   "GLUPStateBlock." 
         * \return a pointer to where the variable is represented in
         *  client side.
         */
        virtual Memory::pointer get_state_variable_address(const char* name);

        /**
         * \brief Gets the uniform state.
         * \return a reference to the uniform state
         */
        UniformState& uniform_state() {
            return uniform_state_;
        }

        /**
         * \brief Gets the uniform state.
         * \return a const reference to the uniform state
         */
        const UniformState& uniform_state() const {
            return uniform_state_;
        }

        /**
         * \brief Indicates that the OpenGL representation
         *  of the uniform state is no longer in sync with
         *  the local copy.
         */
        void flag_uniform_buffer_as_dirty() {
            uniform_buffer_dirty_ = true;
        }


        /**
         * \brief Indicates that cached lighting information 
         *  needs to be recomputed.
         */
        void flag_lighting_as_dirty() {
            uniform_buffer_dirty_ = true;            
            lighting_dirty_ = true;
        }

        /**
         * \brief Indicates that cached matrix information 
         *  needs to be recomputed.
         */
        void flag_matrices_as_dirty() {
            uniform_buffer_dirty_ = true;            
            matrices_dirty_ = true;
        }

        /**
         * \brief Gets a pointer to the values of the matrix at the
         *  top of a given stack.
         * \param[in] matrix name of the stack, one of GLUP_MODELVIEW_MATRIX,
         *   GLUP_PROJECTION_MATRIX, GLUP_TEXTURE_MATRIX
         */
        GLUPfloat* get_matrix(GLUPmatrix matrix) {
            geo_debug_assert(matrix < 3);
            return matrix_stack_[matrix].top();
        }

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/vertex_shader_preamble.h.
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_vertex_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/fragment_shader_preamble.h
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_fragment_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/geometry_shader_preamble.h
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_geometry_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/tess_control_shader_preamble.h
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_tess_control_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/tess_evaluation_shader_preamble.h
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_tess_evaluation_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );
        
        
        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/toggles.h
         * \details The toggles are generated in function of the parameters
         *   of the previous call to setup_shaders_source_for_toggles()
         *  current configuration defined by prepare_sources_for_toggles()
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_toggles_pseudo_file(
            std::vector<GLSL::Source>& sources            
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/primitive.h
         * \details The current primitive is defined by the argument of 
         *  the previous call of setup_shaders_source_for_primitive().
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_primitive_pseudo_file(
            std::vector<GLSL::Source>& sources            
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/marching_cells.h
         * \details The current primitive is defined by the argument of 
         *  the previous call of setup_shaders_source_for_primitive().
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_marching_cells_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Sets the string that describes the settings of
         *  the toggles for a given configuration.
         * \param[in] toggles_state an unsigned integer, with its bits
         *  corresponding to the state of each toggle
         * \param[in] toggles_undetermined an unsigned integer, with its bits
         *  set if the corresponding toggle state needs to be determined
         *  dynamically from GLUP state
         */
        void setup_shaders_source_for_toggles(
            GLUPbitfield toggles_state,
            GLUPbitfield toggles_undetermined=0
        );

        /**
         * \brief Sets the configurable GLSL sources for a given 
         *  primitive type.
         * \details This function needs to be called before compiling
         *  the GLSL program.
         * \param[in] primitive the GLUP primitive
         */
        virtual void setup_shaders_source_for_primitive(
            GLUPprimitive primitive
        );

	/**
	 * \brief Gets the immediate state.
	 * \return a reference to the immediate state.
	 */
	ImmediateState& immediate_state() {
	    return immediate_state_;
	}

        /**
         * \brief Flushes the immediate mode buffers.
         */
        virtual void flush_immediate_buffers();
            
    protected:

        /**
         * \brief Gets the MarchingCell that corresponds to the
         *  current primitive.
         * \details The current primitive is defined by the argument of 
         *  the previous call of setup_shaders_source_for_primitive().
         * \return A const reference to the current MarchingCell.
         */
        const MarchingCell& get_marching_cell() const;
        
        /**
         * \brief Tests whether an OpenGL extension is supported.
         * \param[in] extension the name fo the extension to be tested.
         * \details This function needs to be called before starting using
         *   the extension, even if you are sure that it is supported. In
         *   particular, WebGL specification requires that.
         * \retval true if the extension is supported.
         * \retval false otherwise.
         */
        bool extension_is_supported(const std::string& extension);
        
        /**
         * \brief Gets the name of a primitive by GLUPprimitive.
         * \param[in] prim a GLUPprimitive
         * \return the name of the primitive, as a const char pointer
         */
        const char* glup_primitive_name(GLUPprimitive prim);
        
        /**
         * \brief This function is called before starting to
         *  render primitives. It is called by begin(), draw_arrays()
         *  and draw_elements().
         * \details Some primitives require to change some
         *  parameters in OpenGL. For instance, when we use
         *  GL_PATCH to gather the vertices of hexahedra and
         *  tetrahedra, the number of vertices per patch needs
         *  to be specified to OpenGL.
         */
        virtual void prepare_to_draw(GLUPprimitive primitive);


        /**
         * \brief This function is called right after 
         *  rendering primitives. It is called by end(), draw_arrays()
         *  and draw_elements().
         * \details Default implementation does nothing. This function
         *  is meant to be overloaded by derived Context classes.
         */
        virtual void done_draw(GLUPprimitive primitive);
        
        /**
         * \brief Initializes the representation of the uniform state.
         */
        virtual void setup_state_variables();


        /**
         * \brief Set-ups the buffers for immediate rendering.
         * \details This creates VBOs and the VAO.
         */
        virtual void setup_immediate_buffers();

        /**
         * \brief Sends all the active immediate buffers to the GPU.
         * \details Overwrites the VBOs with the contents of the buffers.
         */
        virtual void stream_immediate_buffers();

        /**
         * \brief Setups the programs and VAOs used for each primitive.
         */
        virtual void setup_primitives();

        /**
         * \brief Setups GLSL programs for points.
         */
        virtual void setup_GLUP_POINTS();

        /**
         * \brief Setups GLSL programs for lines.
         */
        virtual void setup_GLUP_LINES();

        /**
         * \brief Setups GLSL programs for triangles.
         */
        virtual void setup_GLUP_TRIANGLES();

        /**
         * \brief Setups GLSL programs for quads.
         */
        virtual void setup_GLUP_QUADS();

        /**
         * \brief Setups GLSL programs for tetrahedra.
         */
        virtual void setup_GLUP_TETRAHEDRA();

        /**
         * \brief Setups GLSL programs for hexahedra.
         */
        virtual void setup_GLUP_HEXAHEDRA();

        /**
         * \brief Setups GLSL programs for prisms.
         */
        virtual void setup_GLUP_PRISMS();

        /**
         * \brief Setups GLSL programs for pyramids.
         */
        virtual void setup_GLUP_PYRAMIDS();

        /**
         * \brief Setups GLSL programs for connectors.
         */
        virtual void setup_GLUP_CONNECTORS();

        /**
         * \brief Setups GLSL programs for spheres.
         */
        virtual void setup_GLUP_SPHERES();
	
        /**
         * \brief Initializes the PrimitiveInfo associated with a 
         *  given GLUP primitive.
         * \param[in] glup_primitive the GLUP primitive.
         * \param[in] gl_primitive the GL primitive used by the implementation
         * \param[in] program the GLSL program used by the implementation
         * \param[in] bind_attrib_loc_and_link if true, binds attribute 
         *  location and links the shader
         */
        virtual void set_primitive_info(
            GLUPprimitive glup_primitive, GLenum gl_primitive, GLuint program,
            bool bind_attrib_loc_and_link = true
        );

        /**
         * \brief Initializes the PrimitiveInfo associated with a 
         *  given GLUP primitive in vertex-gather mode.
         * \details In vertex-gather mode, all the coordinates of all vertices
         *  and all attributes of the primitive are gathered into a small
         *  number of vertices. This is required by 
         *  primitives that have a number of vertices that corresponds to no 
         *  existing OpenGL primitive (i.e., hexahedron and pyramid). 
         * \param[in] glup_primitive the GLUP primitive.
         * \param[in] gl_primitive the GL primitive used to display the GLUP
         *   primitive. The number of vertices of the GL primitive needs to
         *   be a divisor of the number of vertices of the GLUP primitive.
         * \param[in] program the GLSL program used by the implementation
         */
        virtual void set_primitive_info_vertex_gather_mode(
            GLUPprimitive glup_primitive, GLenum gl_primitive, GLuint program
        );

        /**
         * \brief Initializes the PrimitiveInfo associated with a 
         *  given GLUP primitive in immediate mode when an element index
         *  buffer is required.
         * \details An element index buffer is required when geometry
         *  shaders are not supported, for instance when using OpenGL ES in
         *  webGL.
         * \param[in] glup_primitive the GLUP primitive.
         * \param[in] gl_primitive the GL primitive used to display the GLUP
         *   primitive. 
         * \param[in] program the GLSL program used by the implementation.
         * \param[in] nb_elements_per_glup_primitive the number of element
         *  indices for each glup primitive. For instance, when drawing
         *  GLUP tetrahedra using OpenGL triangles, there are 4*3 = 12
         *  elements per primitive.
         * \param[in] element_indices a pointer to an array of 
         *  nb_elements_per_glup_primitive integers that encode the
         *  indexing of one element. This array is replicated and shifted
         *  to generate the element index buffer. 
         */
        virtual void set_primitive_info_immediate_index_mode(
            GLUPprimitive glup_primitive, GLenum gl_primitive, GLuint program,
            index_t nb_elements_per_glup_primitive,
            index_t* element_indices
        );
        
        /**
         * \brief Copies GLUP uniform state to OpenGL
         *  if required.
         */
        void update_uniform_buffer() {
            if(uniform_buffer_dirty_) {
                do_update_uniform_buffer();
            }
        }

        /**
         * \brief Copies GLUP uniform state to OpenGL.
         * \details This is the implementation of 
         *  update_uniform_buffer().
         */
        virtual void do_update_uniform_buffer();
        
        /**
         * \brief Updates the matrices in the uniform state
         *  from the matrices in the stacks.
         */
        virtual void update_matrices();

        /**
         * \brief Updates the lighting in the uniform state.
         * \details Computes the half vector from the lighting
         *  vector.
         */
        virtual void update_lighting();

        /**
         * \brief Updates the base picking id and sends it to
         *  OpenGL.
         */
        virtual void update_base_picking_id(GLint new_value);

        /**
         * \brief Gets the GLSL declaration of the constant that
         *  indicates the current primitive.
         * \return a string with the GLSL declaration.
         */
        std::string primitive_declaration(GLUPprimitive prim) const;
        
        /**
         * \brief Sets the string that describes the settings of
         *  the toggles for a given configuration.
         * \param[in] toggles_config the identifier of the toggles
         *  configurations, used to index the GLSL program in the
         *  PrimitiveInfo class
         */
        void setup_shaders_source_for_toggles_config(
            PrimitiveInfo::ShaderKey toggles_config
        ) {
            if(toggles_config == (1 << GLUP_PICKING)) {
                setup_shaders_source_for_toggles(
                    (1 << GLUP_PICKING),  // picking=true
                    (1 << GLUP_CLIPPING)  // clipping=undecided (use state)
                );
            } else {
                setup_shaders_source_for_toggles(GLUPbitfield(toggles_config));
            }
        }
        
        /**
         * \brief Updates the toggles_config_ state variable from
         *  the individual state of each toggle.
         */
        void update_toggles_config();

        /**
         * \brief Creates the GLSL shader that corresponds to the
         *  specified primitive and current toggles configuration if
         *  not already initialized.
         * \param[in] primitive the primitive to be displayed
         */
        void create_program_if_needed(GLUPprimitive primitive);

        /**
         * \brief Shrinks the cells in the immediate buffer.
         * \details Applies the shrinking factor (state variable
         *   "cells_shrink") to all the cells stored in the current
         *   immediate buffer. Since there is no function to query
         *   the content of the current buffer, modidying it is 
         *   acceptable. This function is used by derived classes
         *   (VanillaGL and ES2) that cannot shrink the cells 
         *   with a shader.
         */
        void shrink_cells_in_immediate_buffers();

        /**
         * \brief Creates a buffer for uniform variables for
         *  implementations that do not support uniform buffer
         *  objects.
         * \details This function is uses by VanillaGL and ES2.
         */
        void create_CPU_side_uniform_buffer();

        /**
         * \brief Binds the VBOs associated with the immediate
         *  state buffers to the currently bound VAO.
         */
        void bind_immediate_state_buffers_to_VAO();

        /**
         * \brief Updates v_is_visible_[] according to
         *  current clipping plane.
         * \details Used by implementations of Context that
         *  do not support clipping by shaders (VanillaGL and
         *  ES2).
         */
        void classify_vertices_in_immediate_buffers();

        /**
         * \brief Tests whether the cell starting at a given vertex
         *  in the immediate buffer is clipped, according to current
         *  clipping mode and current primitive type.
         * \param[in] first_v index of the first vertex of the cell in
         *  the immediate buffer
         * \retval true if the cell starting at \p first_v in the 
         *  immediate buffer is clipped-out
         * \retval false otherwise
         */
        bool cell_is_clipped(index_t first_v);


        /**
         * \brief Assemble the configuration code of a primitive
         *  relative to the clipping plane.
         * \param[in] first_v index of the first vertex of the 
         *  primitive in the immediate buffer
         * \param[in] nb_v number of vertices of the primitive
         * \return an integer with the i-th bit set if vertex i
         *  is visible, and unset if it is clipped.
         */
        index_t get_config(index_t first_v, index_t nb_v) {
            index_t result = 0;
            for(index_t lv=0; lv<nb_v; ++lv) {
                if(v_is_visible_[first_v+lv]) {
                    result = result | (1u << lv);
                }
            }
            return result;
        }

        /**
         * \brief Computes the intersection between the clipping plane and
         *  a segment.
         * \param[in] v1 index of the first extremity of the segment in the
         *  immediate buffer
         * \param[in] v2 index of the second extremity of the segment in the
         *  immediate buffer
         * \param[in] vi index of where to wrote the intersection in the 
         *  isect_xxx arrays
         */
        void compute_intersection(index_t v1, index_t v2, index_t vi) {
            const GLUPfloat* eqn = world_clip_plane_;
            const GLUPfloat* p1 = immediate_state_.buffer[0].element_ptr(v1);
            const GLUPfloat* p2 = immediate_state_.buffer[0].element_ptr(v2);
            
            GLUPfloat t = -eqn[3] -(
                eqn[0]*p1[0] +
                eqn[1]*p1[1] +
                eqn[2]*p1[2]
            );

            GLUPfloat d =
                eqn[0]*(p2[0]-p1[0]) +
                eqn[1]*(p2[1]-p1[1]) +
                eqn[2]*(p2[2]-p1[2]) ;
            
            if(fabs(double(d)) < 1e-6) {
                t = 0.5f;
            } else {
                t /= d;
            }

            GLUPfloat s = 1.0f - t;
            
            isect_vertex_attribute_[0][4*vi+0] = s*p1[0] + t*p2[0];
            isect_vertex_attribute_[0][4*vi+1] = s*p1[1] + t*p2[1];
            isect_vertex_attribute_[0][4*vi+2] = s*p1[2] + t*p2[2];
            isect_vertex_attribute_[0][4*vi+3] = 1.0f;

            for(index_t i=1; i<3; ++i) {
                if(immediate_state_.buffer[i].is_enabled()) {
                    const GLUPfloat* a1 =
                        immediate_state_.buffer[i].element_ptr(v1);
                    const GLUPfloat* a2 =
                        immediate_state_.buffer[i].element_ptr(v2);
                    isect_vertex_attribute_[i][4*vi+0] = s*a1[0] + t*a2[0];
                    isect_vertex_attribute_[i][4*vi+1] = s*a1[1] + t*a2[1];
                    isect_vertex_attribute_[i][4*vi+2] = s*a1[2] + t*a2[2];
                    isect_vertex_attribute_[i][4*vi+3] = s*a1[3] + t*a2[3]; 
                }
            }
        }
        
        /**
         * \brief Copies the uniform state from client-side 
         *  memory into the currently bound program, or does
         *  nothing if uniform buffer objects are supported.
         */
        virtual void copy_uniform_state_to_current_program();
        
        /**
         * \brief A wrapper around glUseProgram that tests whether
         *  uniform state needs to be sent to the program.
         * \details Each time a different program is used, the
         *  uniform state can be sent to it through the virtual
         *  function update_program_state(). If UBOs are supported,
         *  update_program_state() does nothing.
         */
        void use_program(GLuint program) {
            if(program != 0 && program != latest_program_) {
                glUseProgram(program);
                latest_program_ = program;                
                copy_uniform_state_to_current_program();
            } else {
                glUseProgram(program);
            }
        }

        /**
         * \brief Creates a vertex buffer object with 16 bits integers
         *  between 0 and 65535.
         * \details It is used to emulate gl_VertexID if GLSL does not
         *  support it.
         */
        void create_vertex_id_VBO();

        static void initialize();
        
    protected:
        
        // OpenGL Uniform state.
        GLuint default_program_;
        GLuint uniform_buffer_;
        GLuint uniform_binding_point_;
        GLint  uniform_buffer_size_;
        bool uniform_buffer_dirty_;

        // C++ Uniform state.
        Memory::byte* uniform_buffer_data_;        
        UniformState uniform_state_;

        bool lighting_dirty_;
        
        // Matrix stacks.
        GLUPmatrix matrix_mode_;
        MatrixStack matrix_stack_[3];
        bool matrices_dirty_;

        // Immediate mode buffers.
        ImmediateState immediate_state_;

        // Primitive informations (i.e., how to
        // draw a primitive of a given type).
        vector<PrimitiveInfo> primitive_info_;

        // The marching cells, for computing
        // intersections when clipping mode
        // is GLUP_CLIP_SLICE_CELLS
        MarchingCell marching_tet_;
        MarchingCell marching_hex_;
        MarchingCell marching_prism_;
        MarchingCell marching_pyramid_;
        MarchingCell marching_connector_;
        
        GLuint user_program_;
        
	PrimitiveInfo::ShaderKey toggles_config_;
        
        GLUPprimitive primitive_source_;
        GLUPbitfield toggles_source_state_;
        GLUPbitfield toggles_source_undetermined_;
        
        bool precompile_shaders_;

        bool use_core_profile_;
        bool use_ES_profile_;

        /**
         * \brief Cached pointer to uniform state variable.
         * \details It is initialized by create_GPU_side_uniform_buffer(),
         *  used only by VanillaGL and ES2 implementations.
         */
        GLUPfloat* world_clip_plane_;

        /**
         * \brief Used by GPU-side uniform buffer.
         * \details It is initialized by create_GPU_side_uniform_buffer(),
         *  used only by VanillaGL and ES2 implementations.
         */
        std::map<std::string, GLsizei> variable_to_offset_;

        /**
         * \brief Indicates for a given vertex whether it is clipped or
         *  is visible, according to the current clipping plane.
         * \details Used when clipping is done by software.
         */
        bool v_is_visible_[IMMEDIATE_BUFFER_SIZE];

        /**
         * \brief computed intersections.
         * \details Used when clipping mode is GLUP_CLIP_SLICE_CELLS and
         *  clipping is done by software.
         */
        GLUPfloat isect_vertex_attribute_[3][12*4];
        
        /**
         * \brief Latest used GLSL program.
         * \details Used to check whether it changed and whether some
         *  uniform variables need to be sent to it.
         */
        GLuint latest_program_;

        /**
         * \brief A vertex buffer object with 65536 16 bits integers.
         * \details It is used to emulate gl_VertexID in shaders.
         */
        GLuint vertex_id_VBO_;
    };

    /*********************************************************************/
}

#endif
