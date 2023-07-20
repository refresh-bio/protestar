#pragma once

#include <cstring>
#include <algorithm>

using namespace std;

// ************************************************************************************
// Class for 2D matrix
template <typename T> class matrix2d_t
{
	size_t n_rows, n_cols;
	T* data;

	void reallocate()
	{
		if (data)
			delete[] data;

		data = new T[n_rows * n_cols];
	}

	void deallocate()
	{
		if (data)
			delete[] data;

		data = nullptr;
	}

public:
	matrix2d_t() :
		n_rows(0), n_cols(0), data(nullptr)
	{}

	matrix2d_t(size_t _n_rows, size_t _n_cols) :
		n_rows(_n_rows), n_cols(_n_cols), data(nullptr)
	{
		reallocate();
	}

	matrix2d_t(size_t _size) :
		n_rows(_size), n_cols(_size), data(nullptr)
	{
		reallocate();
	}

	~matrix2d_t()
	{
		deallocate();
	};

	matrix2d_t(const matrix2d_t&rhs)
	{
		if (*this != rhs)
		{
			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;

			reallocate();

			copy_n(rhs.data, n_rows * n_cols, this->data);
		}
	} 

	matrix2d_t(matrix2d_t&&rhs)
	{
		if (*this != rhs)
		{
			deallocate();

			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			data = rhs.data;

			rhs.n_rows = 0;
			rhs.n_cols = 0;
			rhs.data = nullptr;
		}
	}

	matrix2d_t& operator=(const matrix2d_t&rhs)
	{
		if (*this != rhs)
		{
			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;

			reallocate();
			copy_n(rhs.data, n_rows * n_cols, this->data);
		}

		return *this;
	}

	matrix2d_t& operator=(matrix2d_t&& rhs)
	{
		if (*this != rhs)
		{
			deallocate();

			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			data = rhs.data;

			rhs.n_rows = 0;
			rhs.n_cols = 0;
			rhs.data = nullptr;
		}

		return *this;
	}

	void resize(size_t _n_rows, size_t _n_cols)
	{
		n_rows = _n_rows;
		n_cols = _n_cols;
		reallocate();
	}

	void resize(size_t _size)
	{
		n_rows = _size;
		n_cols = _size;
		reallocate();
	}

	void clear()
	{
		n_rows = 0;
		n_cols = 0;
		if (data)
			delete[] data;
		data = nullptr;
	}

	void set_to_zero()
	{
		fill_n(data, n_rows * n_cols, 0);		
	}

	T* ptr(size_t row, size_t col) 
	{
		return data + row * n_cols + col;
	}

	T* ptr(size_t row) 
	{
		return data + row * n_cols;
	}

	const T* ptr(size_t row, size_t col) const
	{
		return data + row * n_cols + col;
	}

	const T* ptr(size_t row) const
	{
		return data + row * n_cols;
	}

	const T& operator()(size_t row, size_t col) const
	{
		return data[row * n_cols + col];
	}

	T& operator()(size_t row, size_t col)
	{
		return data[row * n_cols + col];
	}

	T max_value() const
	{
		return *max_element(data, data + n_rows * n_cols);
	}

	size_t get_n_rows() const
	{
		return n_rows;
	}

	size_t get_n_cols() const
	{
		return n_cols;
	}

	T* raw_ptr() const
	{
		return data;
	}

	size_t raw_size() const
	{
		return n_cols * n_rows;
	}
};

// ************************************************************************************
// Class for 2D matrix with some frame (unused but allocated memory for easier handling of boundary cases)
template <typename T, unsigned int FRAME> class matrix2d_frame_t
{
	int n_rows, n_cols;
	int n_frows, n_fcols;
	size_t fdata_size;
	T* fdata;
	T* data;

	void reallocate()
	{
		if (fdata)
		{
			if ((size_t) n_frows * (size_t) n_fcols > fdata_size)
			{
				fdata = (T*)realloc(fdata, n_frows * n_fcols * sizeof(T));
				fdata_size = n_frows * n_fcols;
			}
		}
		else
		{
			fdata = (T*)malloc(n_frows * n_fcols * sizeof(T));
			fdata_size = (size_t)n_frows * (size_t)n_fcols;
		}

		data = fdata + FRAME * n_fcols + FRAME;
	}

	void deallocate()
	{
		if (fdata)
			free(fdata);

		fdata = nullptr;
		data = nullptr;
		fdata_size = 0;
	}

public:
	matrix2d_frame_t() :
		n_rows(0), n_cols(0), n_frows(0), n_fcols(0), fdata_size(0), fdata(nullptr), data(nullptr)
	{}

	matrix2d_frame_t(int _n_rows, int _n_cols) :
		n_rows(_n_rows), n_cols(_n_cols), 
		n_frows(n_rows + 2 * FRAME), n_fcols(n_cols + 2* FRAME),
		fdata_size(0),
		fdata(nullptr), data(nullptr)
	{
		reallocate();
	}

	~matrix2d_frame_t()
	{
		deallocate();
	};

	matrix2d_frame_t(const matrix2d_frame_t& rhs)
	{
		if (*this != rhs)
		{
			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			n_frows = rhs.n_frows;
			n_fcols = rhs.nfcols;

			reallocate();

			copy_n(rhs.fdata, n_frows * n_fcols, this->fdata);
			data = fdata + FRAME * n_fcols + FRAME;
		}
	}

	matrix2d_frame_t(matrix2d_frame_t&& rhs)
	{
		if (*this != rhs)
		{
			deallocate();

			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			n_frows = rhs.n_frows;
			n_fcols = rhs.nfcols;
			data = rhs.data;
			fdata = rhs.fdata;
			fdata_size = rhs.fdata_size;

			rhs.n_rows = 0;
			rhs.n_cols = 0;
			rhs.n_frows = 0;
			rhs.n_fcols = 0;
			rhs.data = nullptr;
			rhs.fdata = nullptr;
			rhs.fdata_size = 0;
		}
	}

	matrix2d_frame_t& operator=(const matrix2d_frame_t& rhs)
	{
		if (*this != rhs)
		{
			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			n_frows = rhs.n_frows;
			n_fcols = rhs.nfcols;

			reallocate();
			copy_n(rhs.data, n_frows * n_fcols, this->fdata);
		}

		return *this;
	}

	matrix2d_frame_t& operator=(matrix2d_frame_t&& rhs)
	{
		if (*this != rhs)
		{
			deallocate();

			n_rows = rhs.n_rows;
			n_cols = rhs.n_cols;
			n_frows = rhs.n_frows;
			n_fcols = rhs.n_fcols;
			data = rhs.data;
			fdata = rhs.fdata;
			fdata_size = rhs.fdata_size;

			rhs.n_rows = 0;
			rhs.n_cols = 0;
			rhs.n_frows = 0;
			rhs.n_fcols = 0;
			rhs.data = nullptr;
			rhs.fdata = nullptr;
			rhs.fdata_size = 0;
		}

		return *this;
	}

	void resize(size_t _n_rows, size_t _n_cols)
	{
		n_rows = (int) _n_rows;
		n_cols = (int) _n_cols;
		n_frows = n_rows + 2 * FRAME;
		n_fcols = n_cols + 2 * FRAME;

		reallocate();
	}

	void clear()
	{
		n_rows = 0;
		n_cols = 0;
		n_frows = 0;
		n_fcols = 0;
		if (fdata)
			//			delete[] fdata;
			free(fdata);
		fdata = nullptr;
		data = nullptr;
		fdata_size = 0;
	}

	void set_to_zero()
	{
//		fill_n(fdata, n_frows * n_fcols, 0);
		memset(fdata, 0, n_frows * n_fcols * sizeof(T));
	}
	
	void set_frame_to_zero()
	{
		T* ptr = fdata;

		memset(ptr, 0, FRAME * n_fcols * sizeof(T));

		ptr += FRAME * n_fcols;

		for (int i = 0; i < n_rows; ++i)
		{
			memset(ptr, 0, FRAME * sizeof(T));
			ptr += FRAME + n_cols;
			memset(ptr, 0, FRAME * sizeof(T));
			ptr += FRAME;
		}

		memset(ptr, 0, FRAME * n_fcols * sizeof(T));
	}

	T* ptr(int row, int col)
	{
		return data + row * n_fcols + col;
	}

	T* ptr(int row)
	{
		return data + row * n_fcols;
	}

	const T* ptr(int row, int col) const
	{
		return data + row * n_fcols + col;
	}

	const T* ptr(int row) const
	{
		return data + row * n_fcols;
	}

	const T& operator()(int row, int col) const
	{
		return data[row * n_fcols + col];
	}

	T& operator()(int row, int col)
	{
		return data[row * n_fcols + col];
	}

	T max_value() const
	{
		T mv = (T)data[0];

//		for (size_t i = 0; i < n_rows; ++i)
		for (int i = 0; i < n_rows; ++i)
		{
			T x = *max_element(data + i * n_fcols, data + i * n_fcols + n_cols);
			if (x > mv)
				mv = x;
		}

		return mv;
	}

	size_t get_n_rows() const
	{
		return n_rows;
	}

	size_t get_n_cols() const
	{
		return n_cols;
	}

	size_t get_frame() const
	{
		return FRAME;
	}

	T* raw_ptr() const
	{
		return fdata;
	}

	size_t raw_size() const
	{
		return n_fcols * n_frows;
	}
};

// EOF
