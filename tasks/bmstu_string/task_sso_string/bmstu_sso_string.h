#pragma once

#include <exception>
#include <iostream>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
	static constexpr size_t SSO_CAPACITY =
		(sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

	struct LongString
	{
		T* ptr;
		size_t size;
		size_t capacity;
	};

	struct ShortString
	{
		T buffer[SSO_CAPACITY + 1];
		unsigned char size;
	};

	union Data
	{
		LongString long_str;
		ShortString short_str;
	};

	Data data_;
	bool is_long_;

	bool is_long() const { return is_long_; }

	T* get_ptr()
	{
		if (is_long_)
		{
			return data_.long_str.ptr;
		}
		else
		{
			return data_.short_str.buffer;
		}
	}

	const T* get_ptr() const
	{
		if (is_long_)
		{
			return data_.long_str.ptr;
		}
		else
		{
			return data_.short_str.buffer;
		}
	}

	size_t get_size() const
	{
		if (is_long_)
		{
			return data_.long_str.size;
		}
		else
		{
			return data_.short_str.size;
		}
	}

	size_t get_capacity() const
	{
		if (is_long_)
		{
			return data_.long_str.capacity;
		}
		else
		{
			return SSO_CAPACITY;
		}
	}

   public:
	basic_string()
	{
		is_long_ = false;
		data_.short_str.buffer[0] = T(0);
		data_.short_str.size = 0;
	}

	basic_string(size_t size)
	{
		if (size > SSO_CAPACITY)
		{
			is_long_ = true;
			data_.long_str.size = size;
			data_.long_str.capacity = size * 2;
			data_.long_str.ptr = new T[data_.long_str.capacity];

			for (size_t i = 0; i < size; i++)
			{
				data_.long_str.ptr[i] = T(' ');
			}
			data_.long_str.ptr[size] = T(0);
		}
		else
		{
			is_long_ = false;
			data_.short_str.size = size;
			for (size_t i = 0; i < size; i++)
			{
				data_.short_str.buffer[i] = T(' ');
			}
			data_.short_str.buffer[size] = T(0);
		}
	}

	basic_string(std::initializer_list<T> il)
	{
		size_t size = il.size();
		if (size > SSO_CAPACITY)
		{
			is_long_ = true;
			data_.long_str.size = size;
			data_.long_str.capacity = size * 2;
			data_.long_str.ptr = new T[data_.long_str.capacity];
		}
		else
		{
			is_long_ = false;
			data_.short_str.size = size;
		}
		size_t i = 0;
		for (T value : il)
		{
			get_ptr()[i] = value;
			i++;
		}
		get_ptr()[size] = T(0);
	}

	basic_string(const T* c_str)
	{
		size_t len = strlen_(c_str);

		if (len > SSO_CAPACITY)
		{
			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len * 2;
			data_.long_str.ptr = new T[data_.long_str.capacity];
		}
		else
		{
			is_long_ = false;
			data_.short_str.size = len;
		}

		for (size_t i = 0; i < len; i++)
		{
			get_ptr()[i] = c_str[i];
		}
		get_ptr()[len] = T(0);
	}

	basic_string(const basic_string& other)
	{
		is_long_ = other.is_long_;
		if (is_long_)
		{
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;
			data_.long_str.ptr = new T[data_.long_str.capacity];
		}
		else
		{
			data_.short_str.size = other.data_.short_str.size;
		}

		for (size_t i = 0; i <= other.size(); i++)
		{
			get_ptr()[i] = other.get_ptr()[i];
		}
	}

	basic_string(basic_string&& dying) noexcept
	{
		is_long_ = dying.is_long_;

		if (is_long_)
		{
			data_.long_str.size = dying.data_.long_str.size;
			data_.long_str.capacity = dying.data_.long_str.capacity;
			data_.long_str.ptr = dying.data_.long_str.ptr;
			dying.data_.long_str.ptr = nullptr;
			dying.is_long_ = false;
		}

		else
		{
			data_.short_str.size = dying.data_.short_str.size;
			for (size_t i = 0; i <= data_.short_str.size; i++)
			{
				data_.short_str.buffer[i] = dying.data_.short_str.buffer[i];
			}
			dying.data_.short_str.buffer[0] = T(0);
			dying.data_.short_str.size = 0;
		}
	}

	~basic_string() { clean_(); }

	const T* c_str() const { return get_ptr(); }

	size_t size() const { return get_size(); }

	bool is_using_sso() const { return !is_long_; }

	size_t capacity() const { return get_capacity(); }

	basic_string& operator=(basic_string&& other) noexcept
	{
		clean_();

		if (other.is_long_)
		{
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;
			data_.long_str.ptr = other.data_.long_str.ptr;
			other.data_.long_str.ptr = nullptr;
			other.data_.long_str.size = 0;
		}

		else
		{
			data_.short_str.size = other.data_.short_str.size;
			for (size_t i = 0; i <= data_.short_str.size; i++)
			{
				get_ptr()[i] = other.get_ptr()[i];
			}
			other.data_.short_str.buffer[0] = T(0);
			other.data_.short_str.size = 0;
		}

		return *this;
	}

	basic_string& operator=(const T* c_str)
	{
		clean_();
		size_t len = strlen_(c_str);

		if (len > SSO_CAPACITY)
		{
			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len * 2;
			data_.long_str.ptr = new T[len * 2];
		}

		else
		{
			is_long_ = false;
			data_.short_str.size = len;
		}

		for (size_t i = 0; i < len; i++)
		{
			get_ptr()[i] = c_str[i];
		}

		get_ptr()[len] = T(0);

		return *this;
	}

	basic_string& operator=(const basic_string& other)
	{
		clean_();

		is_long_ = other.is_long_;

		if (is_long_)
		{
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;
			data_.long_str.ptr = new T[data_.long_str.capacity];
		}

		else
		{
			data_.short_str.size = other.data_.short_str.size;
		}

		for (size_t i = 0; i <= other.size(); i++)
		{
			get_ptr()[i] = other.get_ptr()[i];
		}

		return *this;
	}

	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		basic_string new_string = basic_string(left);

		new_string += right;

		return new_string;
	}

	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		os << obj.c_str();
		return os;
	}

	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		T ch;
		while (is.get(ch))
		{
			obj += ch;
		}
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t new_size = size() + other.size();

		if (new_size > get_capacity())
		{
			T* new_ptr = new T[new_size + 1];
			for (size_t i = 0; i < size(); i++)
			{
				new_ptr[i] = get_ptr()[i];
			}
			clean_();
			data_.long_str.ptr = new_ptr;
			data_.long_str.capacity = new_size * 2;
			data_.long_str.size = new_size;
			is_long_ = true;
		}

		for (size_t i = 0; i <= other.size(); i++)
		{
			get_ptr()[new_size - other.size() + i] = other.get_ptr()[i];
		}

		return *this;
	}

	basic_string& operator+=(T symbol)
	{
		size_t new_size = size() + 1;

		if (size() + 1 > get_capacity())
		{
			T* new_ptr = new T[new_size + 1];
			for (size_t i = 0; i < size(); i++)
			{
				new_ptr[i] = get_ptr()[i];
			}
			clean_();
			data_.long_str.ptr = new_ptr;
			new_ptr = nullptr;
			data_.long_str.capacity = new_size * 2;
			is_long_ = true;
		}

		get_ptr()[new_size - 1] = symbol;
		get_ptr()[new_size] = T(0);

		if (is_long())
		{
			data_.long_str.size = new_size;
		}
		else
		{
			data_.short_str.size = new_size;
		}

		return *this;
	}

	T& operator[](size_t index) noexcept { return get_ptr()[index]; }

	T& at(size_t index)
	{
		if (index >= size())
		{
			throw std::out_of_range("Wrong index");
		}
		return get_ptr()[index];
	}

	T* data() { return get_ptr(); }

   private:
	static size_t strlen_(const T* str)
	{
		if (str == nullptr)
		{
			return 0;
		}
		int c = 0;
		while (*str != T(0))
		{
			++str;
			c++;
		}
		return c;
	}

	void clean_()
	{
		if (is_long_)
		{
			delete[] data_.long_str.ptr;
			data_.long_str.ptr = nullptr;
			data_.long_str.size = 0;
			data_.long_str.capacity = 0;
		}
		else
		{
			data_.short_str.size = 0;
			data_.short_str.buffer[0] = T(0);
		}
	}
};
}  // namespace bmstu