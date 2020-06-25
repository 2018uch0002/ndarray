/*
 * NPArray
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, Hunter Belanger (hunter.belanger@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * */
#ifndef NP_ARRAY_H
#define NP_ARRAY_H

#include<array>
#include<string>
#include<vector>
#include<complex>
#include<cstdint>
#include<cstring>
#include<fstream>
#include<typeinfo>
#include<algorithm>
#include<stdexcept>

//==============================================================================
// Template Class NPArray
template<class T>
class NPArray {
  public:
    //==========================================================================
    // Constructors and Destructors
    NPArray();
    NPArray(std::vector<size_t> init_shape, bool c_continuous=true); 
    NPArray(std::vector<T> data, std::vector<size_t> init_shape,
      bool c_continuous=true); 
    ~NPArray() = default;

    // Static load function
    static NPArray load(std::string fname);

    //==========================================================================
    // Indexing
    
    // Indexing operators for indexing with vector
    T& operator()(const std::vector<size_t>& indices);
    const T& operator()(const std::vector<size_t>& indices) const;

    // Variadic indexing operators
    // Access data with array idicies.
    template <typename... INDS>
    T& operator()(INDS... inds);

    template <typename... INDS>
    const T& operator()(INDS... inds) const;
    
    // Linear Indexing operators
    T& operator[](size_t i);
    const T& operator[](size_t i) const;
    
    //========================================================================== 
    // Constant Methods
    
    // Return vector describing shape of array 
    std::vector<size_t> shape() const;

    // Return number of elements in array
    size_t size() const;
    
    size_t linear_index(const std::vector<size_t>& indices) const;

    template <typename... INDS>
    size_t linear_index(INDS... inds) const;
    
    // Returns true if data is stored as c continuous (row-major order),
    // and false if fortran continuous (column-major order)
    bool c_continuous() const;
    
    // Save array to the file fname.npy
    void save(std::string fname) const;

    //==========================================================================
    // Non-Constant Methods
    
    // Fills entire array with the value provided 
    void fill(T val);
    
    // Will reshape the array to the given dimensions
    void reshape(std::vector<size_t> new_shape);

    // Realocates array to fit the new size.
    // DATA CAN BE LOST IF ARRAY IS SHRUNK
    void reallocate(std::vector<size_t> new_shape);

    //==========================================================================
    // Operators
    // TODO

  private:
    std::vector<T> data_;
    std::vector<size_t> shape_;
    bool c_continuous_;
    size_t dimensions_;
    
    void check_indices(const std::vector<size_t>& indices) const;

    template<size_t D>
    void check_indices(const std::array<size_t, D>& indices) const;
    
    size_t c_continuous_index(const std::vector<size_t>& indices) const;

    size_t fortran_continuous_index(const std::vector<size_t>& indices) const;
    
    template<size_t D>
    inline size_t c_continuous_index(const std::array<size_t, D>& indices) const;

    template<size_t D>
    inline size_t fortran_continuous_index(const std::array<size_t, D>& indices) const;
};

//==============================================================================
// Declarations for NPY functions

// Enum of possible data types which are currently handeled by this Exdir
// implementation.
enum class DType { CHAR, UCHAR, INT16, INT32, INT64, UINT16, UINT32, UINT64,
  FLOAT32, DOUBLE64, COMPLEX64, COMPLEX128 };

// Function which opens file fname, and loads in the binary data into 1D
// array of chars, which must latter be type cast be the user. The char* to
// the data is returned as a reference, along with the number of elements,
// continuits, and element size in bytes. Returned is a vector for the shape.
void load_npy(std::string fname, char*& data_ptr, std::vector<size_t>& shape,
    DType& dtype, bool& c_contiguous);

// Function which writes binary data to a Numpy .npy file.
void write_npy(std::string fname, const char* data_ptr,
    std::vector<size_t> shape, DType dtype, bool c_contiguous);

// Returns the proper DType for a given Numpy dtype.descr string.
DType descr_to_DType(std::string dtype);

// Returns Numpy descr string for given DType
std::string DType_to_descr(DType dtype);

// Returns the number of bytes used to represent the provided DType.
size_t size_of_DType(DType dtype);

// Function to check wether or not host system is little-endian or not
bool system_is_little_endian();

// Function to switch bye order of data for an array which contains
// n_elements, each with element_size bytes. The size of data should
// therefore be n_elements*element_size; if not, this is undefined behavior.
void swap_bytes(char* data, uint64_t n_elements, size_t element_size);

// Swaps the first and second byte pointed to by char* bytes.
void swap_two_bytes(char* bytes);

// Swaps the first four bytes pointed to by char* bytes.
void swap_four_bytes(char* bytes);

// Swaps the first eight bytes pointed to by char* bytes.
void swap_eight_bytes(char* bytes);

// Swaps the first sixteen bytes pointed to by char* bytes.
void swap_sixteen_bytes(char* bytes);

//==============================================================================
// NPArray Implementation
template<class T>
NPArray<T>::NPArray() {}

template<class T>
NPArray<T>::NPArray(std::vector<size_t> init_shape, bool c_continuous) {
  if(init_shape.size() > 0) {
    shape_ = init_shape;
    dimensions_ = shape_.size();

    size_t ne = init_shape[0];
    for(size_t i = 1; i < dimensions_; i++) {
      ne *= init_shape[i]; 
    }

    data_.resize(ne);

    c_continuous_ = c_continuous;

  } else {
    std::string mssg = "NPArray shape vector must have at least one element."; 
    throw std::runtime_error(mssg);
  }
}

template<class T>
NPArray<T>::NPArray(std::vector<T> data, std::vector<size_t> init_shape,
  bool c_continuous) {
  if(init_shape.size() > 0) {
    shape_ = init_shape;
    dimensions_ = shape_.size();

    size_t ne = init_shape[0];
    for(size_t i = 1; i < dimensions_; i++) {
      ne *= init_shape[i]; 
    }

    if(ne != data.size()) {
      std::string mssg = "Shape is incompatible with number of elements provided for NPArray.";
      throw std::runtime_error(mssg);
    }

    data_ = data;

    c_continuous_ = c_continuous;

  } else {
    std::string mssg = "Shape vector must have at least one element for NPArray."; 
    throw std::runtime_error(mssg);
  }
}

template<class T>
NPArray<T> NPArray<T>::load(std::string fname) {
  // Get expected DType according to T
  DType expected_dtype;
  const char* T_type_name = typeid(T).name();

  if(T_type_name == typeid(char).name()) expected_dtype = DType::CHAR;
  else if(T_type_name == typeid(unsigned char).name()) expected_dtype = DType::UCHAR;
  else if(T_type_name == typeid(uint16_t).name()) expected_dtype = DType::UINT16;
  else if(T_type_name == typeid(uint32_t).name()) expected_dtype = DType::UINT32;
  else if(T_type_name == typeid(uint64_t).name()) expected_dtype = DType::UINT64;
  else if(T_type_name == typeid(int16_t).name()) expected_dtype = DType::INT16;
  else if(T_type_name == typeid(int32_t).name()) expected_dtype = DType::INT32;
  else if(T_type_name == typeid(int64_t).name()) expected_dtype = DType::INT64;
  else if(T_type_name == typeid(float).name()) expected_dtype = DType::FLOAT32;
  else if(T_type_name == typeid(double).name()) expected_dtype = DType::DOUBLE64;
  else if(T_type_name == typeid(std::complex<float>).name()) expected_dtype = DType::COMPLEX64;
  else if(T_type_name == typeid(std::complex<double>).name()) expected_dtype = DType::COMPLEX128;
  else {
    std::string mssg = "The datatype is not supported for NPArray."; 
    throw std::runtime_error(mssg);
  }

  // Variables to send to npy function
  char* data_ptr;
  std::vector<T> data_vector;
  std::vector<size_t> data_shape;
  DType data_dtype;
  bool data_c_continuous;

  // Load data into variables
  load_npy(fname, data_ptr, data_shape, data_dtype, data_c_continuous);

  // Ensuire DType variables match
  if(expected_dtype != data_dtype) {
    std::string mssg = "NPArray template datatype does not match specified datatype in npy file."; 
    throw std::runtime_error(mssg);
  }

  if(data_shape.size() < 1) {
    std::string mssg = "Shape vector must have at least one element for NPArray."; 
    throw std::runtime_error(mssg);
  }
  
  // Number of elements
  size_t ne = data_shape[0];
  for(size_t i = 1; i < data_shape.size(); i++) {
    ne *= data_shape[i]; 
  }

  data_vector = {reinterpret_cast<T*>(data_ptr), reinterpret_cast<T*>(data_ptr)+ne};

  // Create NPArray object
  NPArray<T> return_object(data_vector, data_shape);
  return_object.c_continuous_ = data_c_continuous;

  // Free data_ptr
  delete[] data_ptr;

  // Return object
  return return_object;
}

template<class T>
inline T& NPArray<T>::operator()(const std::vector<size_t>& indices) {
  check_indices(indices);

  size_t indx;
  if (c_continuous_) {
    // Get linear index for row-major order
    indx = c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    indx = fortran_continuous_index(indices);
  }
  return data_[indx];
}

template<class T>
inline const T& NPArray<T>::operator()(const std::vector<size_t>& indices) const {
  check_indices(indices);

  size_t indx;
  if (c_continuous_) {
    // Get linear index for row-major order
    indx = c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    indx = fortran_continuous_index(indices);
  }
  return data_[indx];
}

template<class T>
template <typename... INDS>
inline T& NPArray<T>::operator()(INDS... inds) {
  std::array<size_t, sizeof...(inds)> indices{static_cast<size_t>(inds)...};

  check_indices(indices);

  size_t indx;
  if (c_continuous_) {
    // Get linear index for row-major order
    indx = c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    indx = fortran_continuous_index(indices);
  }
  return data_[indx];
}

template<class T>
template <typename... INDS>
inline const T& NPArray<T>::operator()(INDS... inds) const {
  std::array<size_t, sizeof...(inds)> indices{static_cast<size_t>(inds)...};

  check_indices(indices);

  size_t indx;
  if (c_continuous_) {
    // Get linear index for row-major order
    indx = c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    indx = fortran_continuous_index(indices);
  }
  return data_[indx];
}

template<class T>
inline T& NPArray<T>::operator[](size_t i) {
  return data_[i];
}

template<class T>
inline const T& NPArray<T>::operator[](size_t i) const {
  return data_[i];
}

template<class T>
inline std::vector<size_t> NPArray<T>::shape() const {return shape_;}

template<class T>
inline size_t NPArray<T>::size() const {return data_.size();}

template<class T>
inline size_t NPArray<T>::linear_index(const std::vector<size_t>& indices) const {
  check_indices(indices);

  if (c_continuous_) {
    // Get linear index for row-major order
    return c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    return fortran_continuous_index(indices);
  }
}

template<class T>
template <typename... INDS>
inline size_t NPArray<T>::linear_index(INDS... inds) const {
  std::array<size_t, sizeof...(inds)> indices{static_cast<size_t>(inds)...};

  check_indices(indices);

  if (c_continuous_) {
    // Get linear index for row-major order
    return c_continuous_index(indices);
  } else {
    // Get linear index for column-major order
    return fortran_continuous_index(indices);
  }
}
    
template<class T>
inline bool NPArray<T>::c_continuous() const {return c_continuous_;}

template<class T>
void NPArray<T>::save(std::string fname) const {
  // Get expected DType according to T
  DType dtype;
  const char* T_type_name = typeid(T).name();

  if(T_type_name == typeid(char).name()) dtype = DType::CHAR;
  else if(T_type_name == typeid(unsigned char).name()) dtype = DType::UCHAR;
  else if(T_type_name == typeid(uint16_t).name()) dtype = DType::UINT16;
  else if(T_type_name == typeid(uint32_t).name()) dtype = DType::UINT32;
  else if(T_type_name == typeid(uint64_t).name()) dtype = DType::UINT64;
  else if(T_type_name == typeid(int16_t).name()) dtype = DType::INT16;
  else if(T_type_name == typeid(int32_t).name()) dtype = DType::INT32;
  else if(T_type_name == typeid(int64_t).name()) dtype = DType::INT64;
  else if(T_type_name == typeid(float).name()) dtype = DType::FLOAT32;
  else if(T_type_name == typeid(double).name()) dtype = DType::DOUBLE64;
  else if(T_type_name == typeid(std::complex<float>).name()) dtype = DType::COMPLEX64;
  else if(T_type_name == typeid(std::complex<double>).name()) dtype = DType::COMPLEX128;
  else {
    std::string mssg = "The datatype is not supported for NPArray."; 
    throw std::runtime_error(mssg);
  }

  // Write data to file
  write_npy(fname, reinterpret_cast<const char*>(data_.data()), shape_,
            dtype, c_continuous_);
}

template<class T>
inline void NPArray<T>::fill(T val) {
  std::fill(data_.begin(), data_.end(), val); 
}

template<class T>
inline void NPArray<T>::reshape(std::vector<size_t> new_shape) {
  // Ensure new shape has proper dimensions
  if(new_shape.size() < 1) {
    std::string mssg = "Shape vector must have at least one element to"
                       " reshpae NPArray.";
    throw std::runtime_error(mssg);
  } else {
    size_t ne = new_shape[0];
    
    for(size_t i = 1; i < new_shape.size(); i++) {
      ne *= new_shape[i];
    }

    if(ne == data_.size()) {
      shape_ = new_shape;
      dimensions_ = shape_.size();
    } else {
      std::string mssg = "Shape is incompatible with number of elements in"
                         " NPArray.";
      throw std::runtime_error(mssg);
    }
  }
}

template<class T>
void NPArray<T>::reallocate(std::vector<size_t> new_shape) {
  // Ensure new shape has proper dimensions
  if(new_shape.size() < 1) {
    std::string mssg = "Shape vector must have at least one element to"
                       " reallocate NPArray.";
    throw std::runtime_error(mssg);
  } else {
    size_t ne = new_shape[0];
    
    for(size_t i = 1; i < new_shape.size(); i++) {
      ne *= new_shape[i];
    }

    shape_ = new_shape;
    dimensions_ = shape_.size();
    data_.resize(ne);
  }
}

template<class T>
inline void NPArray<T>::check_indices(const std::vector<size_t>& indices) const {
  // Make sure proper number of indices
  if(indices.size() != dimensions_) {
    std::string mssg = "Improper number of indicies provided to NPArray."; 
    throw std::runtime_error(mssg);
  }

  // Make sure all index values are valid
  for(size_t i = 0; i < dimensions_; i++) {
    if(indices[i] >= shape_[i]) {
      std::string mssg = "Invalid index provided to NPArray out of range."; 
      throw std::out_of_range(mssg);
    }
  }
}

template<class T>
template<size_t D>
inline void NPArray<T>::check_indices(const std::array<size_t, D>& indices) const {
  // Make sure proper number of indices
  if(indices.size() != dimensions_) {
    std::string mssg = "Improper number of indicies provided to NPArray."; 
    throw std::runtime_error(mssg);
  }

  // Make sure all index values are valid
  for(size_t i = 0; i < dimensions_; i++) {
    if(indices[i] >= shape_[i]) {
      std::string mssg = "Index provided to NPArray out of range."; 
      throw std::out_of_range(mssg);
    }
  }
}

template<class T>
inline size_t NPArray<T>::c_continuous_index(const std::vector<size_t>& indices) const {
  size_t indx = indices[dimensions_ - 1];
  size_t coeff = 1;

  for(size_t i = dimensions_ - 1; i > 0; i--) {
    coeff *= shape_[i];
    indx += coeff * indices[i-1];
  }

  return indx;
}

template<class T>
inline size_t NPArray<T>::fortran_continuous_index(const std::vector<size_t>& indices)
const {
  size_t indx = indices[0];
  size_t coeff = 1;

  for (size_t i = 0; i < dimensions_-1; i++) {
    coeff *= shape_[i];
    indx += coeff * indices[i+1];
  }

  return indx;
}

template<class T>
template<size_t D>
inline size_t NPArray<T>::c_continuous_index(const std::array<size_t, D>& indices)
const {
  size_t indx = indices[dimensions_ - 1];
  size_t coeff = 1;

  for(size_t i = dimensions_ - 1; i > 0; i--) {
    coeff *= shape_[i];
    indx += coeff * indices[i-1];
  }

  return indx;
}

template<class T>
template<size_t D>
inline size_t NPArray<T>::fortran_continuous_index(const std::array<size_t, D>& indices)
const {
  size_t indx = indices[0];
  size_t coeff = 1;

  for (size_t i = 0; i < dimensions_-1; i++) {
    coeff *= shape_[i];
    indx += coeff * indices[i+1];
  }

  return indx;
}

//==============================================================================
// NPY Function Definitions
inline void load_npy(std::string fname, char*& data_ptr,
              std::vector<size_t>& shape, DType& dtype, bool& c_contiguous) {
  // Open file
  std::ifstream file(fname);

  // Read magic string
  char* magic_string = new char[6];
  file.read(magic_string, 6);

  // Ensure magic string has right value
  if (magic_string[0] != '\x93' || magic_string[1] != 'N' ||
      magic_string[2] != 'U' || magic_string[3] != 'M' ||
      magic_string[4] != 'P' || magic_string[5] != 'Y') {
    std::string mssg = fname + " is an invalid .npy file.";
    throw std::runtime_error(mssg);
  }

  char major_verison = 0x00;
  char minor_version = 0x00;
  file.read(&major_verison, 1);
  file.read(&minor_version, 1);

  uint32_t length_of_header = 0;
  // Do any version checks here if required
  if (major_verison == 0x01) {
    uint16_t length_temp = 0;
    file.read((char*)&length_temp, 2);

    // Value is stored as little endian. If system is big-endian, swap bytes
    if (!system_is_little_endian()) {
      swap_bytes((char*)&length_temp, 2, 1);
    }

    // Cast to main variable
    length_of_header = static_cast<uint32_t>(length_temp);
  } else if (major_verison >= 0x02) {
    uint32_t length_temp = 0;
    file.read((char*)&length_temp, 4);

    // Value is stored as little endian. If system is big-endian, swap bytes
    if (!system_is_little_endian()) {
      swap_bytes((char*)&length_temp, 4, 1);
    }

    // Set to main variable
    length_of_header = length_temp;
  }

  // Array for header, and read in
  char* header_char = new char[length_of_header];
  file.read(header_char, length_of_header);
  std::string header(header_char);

  // Parse header to get continuity
  size_t loc1 = header.find("'fortran_order': ");
  loc1 += 17;
  std::string f_order_string = "";
  size_t i = 0;
  while (true) {
    if (header[loc1 + i] != ' ') {
      if (header[loc1 + i] == ',')
        break;
      else
        f_order_string += header[loc1 + i];
    }
    i++;
  }
  if (f_order_string == "False")
    c_contiguous = true;
  else
    c_contiguous = false;

  // Parse header to get type description
  loc1 = header.find("'descr': ");
  loc1 += 9;
  std::string descr_string = "";
  bool data_is_little_endian = false;
  if (header[loc1 + 1] == '>')
    data_is_little_endian = false;
  else
    data_is_little_endian = true;
  i = 2;
  while (true) {
    if (header[loc1 + i] != ' ') {
      if (header[loc1 + i] != '\'') {
        descr_string += header[loc1 + i];
      } else
        break;
    }
    i++;
  }
  dtype = descr_to_DType(descr_string);
  size_t element_size = size_of_DType(dtype);

  // Parse header to get shape
  loc1 = header.find('(');
  size_t loc2 = header.find(')');
  loc1 += 1;
  // Clear shape vector to ensure it's empty (even though it should already be
  // empty)
  shape.clear();
  if (loc1 == loc2) {
    shape.push_back(1);
  } else {
    std::string temp = "";
    for (i = loc1; i < loc2; i++) {
      if (header[i] != ',')
        temp += header[i];
      else {
        if (temp.size() > 0) {
          shape.push_back(std::stoi(temp));
          temp = "";
        }
      }
    }
    if (temp.size() > 0) {
      shape.push_back(std::stoi(temp));
    }
  }

  // Get number of bytes to be read into system
  uint64_t n_elements = shape[0];
  for (size_t j = 1; j < shape.size(); j++) n_elements *= shape[j];
  uint64_t n_bytes_to_read = n_elements * element_size;
  char* data = new char[n_bytes_to_read];
  file.read(data, n_bytes_to_read);

  // If byte order of data different from byte order of system, swap data bytes
  if (system_is_little_endian() != data_is_little_endian) {
    swap_bytes(data, n_elements, element_size);
  }

  // Set pointer reference
  data_ptr = data;

  // Clear Memory
  delete[] magic_string;
  delete[] header_char;
  file.close();
}

inline void write_npy(std::string fname, const char* data_ptr,
               std::vector<size_t> shape, DType dtype, bool c_contiguous) {
  // Calculate number of elements from the shape
  uint64_t n_elements = shape[0];
  for (size_t j = 1; j < shape.size(); j++) {
    n_elements *= shape[j];
  }

  // Open file
  std::ofstream file(fname);

  // First write magic string
  file << '\x93' << 'N' << 'U' << 'M' << 'P' << 'Y';

  // Next make the header. This is needed to know what version number to use
  std::string header = "{'descr': '";
  // Get system endianness
  if (system_is_little_endian())
    header += "<";
  else
    header += ">";
  header += DType_to_descr(dtype) + "', ";

  // Fortran ordering
  header += "'fortran_order': ";
  if (c_contiguous)
    header += "False, ";
  else
    header += "True, ";

  // Shape
  header += "'shape': (";
  for (const auto& e : shape) {
    header += std::to_string(e) + ",";
  }
  header += "), }";

  // Based on header length, get version
  char major_version;

  uint64_t beginning_length = 6 + 2 + 2 + header.size();
  uint8_t padding_needed = 64 - (beginning_length % 64);
  if (beginning_length + padding_needed > 65535) {
    major_version = 0x02;
    beginning_length += 2;
  } else
    major_version = 0x01;
  // Add padding
  for (uint8_t p = 0; p < padding_needed - 1; p++) header += '\x20';
  header += '\n';

  char minor_version = 0x00;
  file << major_version << minor_version;

  // Inf for len of header
  if (major_version == 0x01) {
    uint16_t len = static_cast<uint16_t>(header.size());
    if (!system_is_little_endian()) {
      swap_two_bytes((char*)&len);
    }
    file << ((char*)&len)[0] << ((char*)&len)[1];
  } else {
    uint32_t len = static_cast<uint32_t>(header.size());
    if (!system_is_little_endian()) {
      swap_four_bytes((char*)&len);
    }
    file << ((char*)&len)[0] << ((char*)&len)[1] << ((char*)&len)[2]
         << ((char*)&len)[3];
  }

  // Write header to file
  file << header.c_str();

  // Write all data to file
  uint64_t n_bytes = n_elements * size_of_DType(dtype);
  for (uint64_t i = 0; i < n_bytes; i++) file << data_ptr[i];

  // Close file
  file.close();
}

inline DType descr_to_DType(std::string dtype) {
  if (dtype == "b1")
    return DType::CHAR;
  else if (dtype == "B1")
    return DType::UCHAR;
  else if (dtype == "i2")
    return DType::INT16;
  else if (dtype == "i4")
    return DType::INT32;
  else if (dtype == "i8")
    return DType::INT64;
  else if (dtype == "u2")
    return DType::UINT16;
  else if (dtype == "u4")
    return DType::UINT32;
  else if (dtype == "u8")
    return DType::UINT64;
  else if (dtype == "f4")
    return DType::FLOAT32;
  else if (dtype == "f8")
    return DType::DOUBLE64;
  else if (dtype == "c8")
    return DType::COMPLEX64;
  else if (dtype == "c16")
    return DType::COMPLEX128;
  else {
    std::string mssg = "Data type " + dtype + " is unknown.";
    throw std::runtime_error(mssg);
  }
}

inline std::string DType_to_descr(DType dtype) {
  if (dtype == DType::CHAR)
    return "b1";
  else if (dtype == DType::UCHAR)
    return "B1";
  else if (dtype == DType::INT16)
    return "i2";
  else if (dtype == DType::INT32)
    return "i4";
  else if (dtype == DType::INT64)
    return "i8";
  else if (dtype == DType::UINT16)
    return "u2";
  else if (dtype == DType::UINT32)
    return "u4";
  else if (dtype == DType::UINT64)
    return "u8";
  else if (dtype == DType::FLOAT32)
    return "f4";
  else if (dtype == DType::DOUBLE64)
    return "f8";
  else if (dtype == DType::COMPLEX64)
    return "c8";
  else if (dtype == DType::COMPLEX128)
    return "c16";
  else {
    std::string mssg = "Unknown DType identifier.";
    throw std::runtime_error(mssg);
  }
}

inline size_t size_of_DType(DType dtype) {
  if (dtype == DType::CHAR)
    return 1;
  else if (dtype == DType::UCHAR)
    return 1;
  else if (dtype == DType::INT16)
    return 2;
  else if (dtype == DType::INT32)
    return 4;
  else if (dtype == DType::INT64)
    return 8;
  else if (dtype == DType::UINT16)
    return 2;
  else if (dtype == DType::UINT32)
    return 4;
  else if (dtype == DType::UINT64)
    return 8;
  else if (dtype == DType::FLOAT32)
    return 4;
  else if (dtype == DType::DOUBLE64)
    return 8;
  else if (dtype == DType::COMPLEX64)
    return 8;
  else if (dtype == DType::COMPLEX128)
    return 16;
  else {
    std::string mssg = "Unknown DType identifier.";
    throw std::runtime_error(mssg);
  }
}

inline bool system_is_little_endian() {
  int x = 1;

  if (((char*)&x)[0] == 0x01)
    return true;
  else
    return false;
}

inline void swap_bytes(char* data, uint64_t n_elements, size_t element_size) {
  // Calculate number of total bytes
  uint64_t number_of_bytes = n_elements * element_size;

  // Iterate through all elements, and swap their bytes
  for (uint64_t i = 0; i < number_of_bytes; i += element_size) {
    if (element_size == 1) {
      // Nothing to do
    } else if (element_size == 2) {
      swap_two_bytes(data + i);
    } else if (element_size == 4) {
      swap_four_bytes(data + i);
    } else if (element_size == 8) {
      swap_eight_bytes(data + i);
    } else if (element_size == 16) {
      swap_sixteen_bytes(data + i);
    } else {
      std::string mssg = "Cannot swap bytes for data types of size " +
                         std::to_string(element_size);
      throw std::runtime_error(mssg);
    }
  }
}

inline void swap_two_bytes(char* bytes) {
  // Temporary array to store original bytes
  char temp[2];

  // Copyr original bytes into temp array
  std::memcpy(&temp, bytes, 2);

  // Set original bytes to new values
  bytes[0] = temp[1];
  bytes[1] = temp[0];
}

inline void swap_four_bytes(char* bytes) {
  // Temporary array to store original bytes
  char temp[4];

  // Copyr original bytes into temp array
  std::memcpy(&temp, bytes, 4);

  // Set original bytes to new values
  bytes[0] = temp[3];
  bytes[1] = temp[2];
  bytes[2] = temp[1];
  bytes[3] = temp[0];
}

inline void swap_eight_bytes(char* bytes) {
  // Temporary array to store original bytes
  char temp[8];

  // Copyr original bytes into temp array
  std::memcpy(&temp, bytes, 8);

  // Set original bytes to new values
  bytes[0] = temp[7];
  bytes[1] = temp[6];
  bytes[2] = temp[5];
  bytes[3] = temp[4];
  bytes[4] = temp[3];
  bytes[5] = temp[2];
  bytes[6] = temp[1];
  bytes[7] = temp[0];
}

inline void swap_sixteen_bytes(char* bytes) {
  // Temporary array to store original bytes
  char temp[16];

  // Copyr original bytes into temp array
  std::memcpy(&temp, bytes, 16);

  // Set original bytes to new values
  bytes[0] = temp[15];
  bytes[1] = temp[14];
  bytes[2] = temp[13];
  bytes[3] = temp[12];
  bytes[4] = temp[11];
  bytes[5] = temp[10];
  bytes[6] = temp[9];
  bytes[7] = temp[8];
  bytes[8] = temp[7];
  bytes[9] = temp[6];
  bytes[10] = temp[5];
  bytes[11] = temp[4];
  bytes[12] = temp[3];
  bytes[13] = temp[2];
  bytes[14] = temp[1];
  bytes[15] = temp[0];
}
#endif // NP_ARRAY_H
