#include <ostream>
#include <stdexcept>
#include <utility>
#include "array_ptr.h"

namespace bmstu
{
template <typename T>
class simple_vector
{
   public:
	class iterator
	{
	   public:
		using iterator_category = std::contiguous_iterator_tag;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		using difference_type = std::ptrdiff_t;

		iterator() = default;

		iterator(const iterator& other) : ptr_(other.ptr_){};

		iterator(std::nullptr_t) noexcept : ptr_(nullptr) {}

		iterator(iterator&& other) noexcept : ptr_(other.ptr_)
		{
			other.ptr_ = nullptr;
		}

		explicit iterator(pointer ptr) : ptr_(ptr) {}

		reference operator*() const { return *ptr_; }

		pointer operator->() const { return ptr_; }

		friend pointer to_address(const iterator& it) noexcept
		{
			return it.ptr_;
		}

		reference operator[](difference_type size) { return *(ptr_ + size); }

		iterator& operator=(const iterator& other)
		{
			ptr_ = other.ptr_;
			return *this;
		};

		iterator& operator=(iterator&& other) noexcept
		{
			if (this != &other)
			{
				ptr_ = other.ptr_;
				other.ptr_ = nullptr;
			}
			return *this;
		}

#pragma region Operators
		iterator& operator++()
		{
			++ptr_;
			return *this;
		}

		iterator& operator--()
		{
			--ptr_;
			return *this;
		}

		iterator operator++(int)
		{
			iterator tmp(*this);
			++ptr_;
			return tmp;
		}

		iterator operator--(int)
		{
			iterator tmp(*this);
			--ptr_;
			return tmp;
		}

		explicit operator bool() const { return ptr_ != nullptr; }

		friend bool operator==(const iterator& lhs, const iterator& rhs)
		{
			return lhs.ptr_ == rhs.ptr_;
		}

		friend bool operator==(const iterator& lhs, std::nullptr_t)
		{
			return lhs.ptr_ == nullptr;
		}

		iterator& operator=(std::nullptr_t) noexcept
		{
			ptr_ = nullptr;
			return *this;
		}

		friend bool operator==(std::nullptr_t, const iterator& rhs)
		{
			return rhs.ptr_ == nullptr;
		}

		friend bool operator!=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.ptr_ != rhs.ptr_;
		}

		iterator operator+(const difference_type& n) const noexcept
		{
			return iterator(ptr_ + n);
		}

		iterator& operator+=(const difference_type& n) noexcept
		{
			this->ptr_ += n;
			return *this;
		}

		iterator operator-(const difference_type& n) const noexcept
		{
			return iterator(ptr_ - n);
		}

		iterator& operator-=(const difference_type& n) noexcept
		{
			this->ptr_ -= n;
			return *this;
		}

		friend difference_type operator-(const iterator& end,
										 const iterator& begin) noexcept
		{
			return end.ptr_ - begin.ptr_;
		}

#pragma endregion
	   private:
		pointer ptr_ = nullptr;
	};

	simple_vector() noexcept = default;

	~simple_vector() { clear(); };

	simple_vector(std::initializer_list<T> init) noexcept
	{
		size_ = init.size();
		capacity_ = init.size();
		if (size_ > 0)
		{
			T* new_ptr = static_cast<T*>(operator new(sizeof(T) * capacity_));
			data_ = array_ptr<T>(new_ptr);
			size_t i = 0;
			for (const auto& item : init)
			{
				new (&data_[i]) T(item);
				i++;
			}
		}
	}

	simple_vector(const simple_vector& other)
	{
		size_ = other.size_;
		capacity_ = other.size_;
		if (size_ > 0)
		{
			T* new_ptr = static_cast<T*>(operator new(sizeof(T) * capacity_));
			data_ = array_ptr<T>(new_ptr);

			for (int i = 0; i < size_; i++)
			{
				new (&data_[i]) T(other.data_[i]);
			}
		}
	}

	simple_vector(simple_vector&& other) noexcept
		: data_(nullptr), size_(0), capacity_(0)
	{
		swap(other);
	}

	simple_vector& operator=(const simple_vector& other)
	{
		if (this != &other)
		{
			simple_vector tmp(other);
			swap(tmp);
		}
		return *this;
	}

	simple_vector(size_t size, const T& value = T{})
	{
		size_ = size;
		capacity_ = size_;

		T* new_data = static_cast<T*>(operator new(sizeof(T) * (size_)));
		data_ = array_ptr<T>(new_data);

		for (int i = 0; i < size; ++i)
		{
			new (&data_[i]) T(value);
		}
	}

	iterator begin() noexcept { return iterator(data_.get()); }

	iterator end() noexcept { return iterator(data_.get() + size_); }

	using const_iterator = iterator;

	const_iterator begin() const noexcept
	{
		return const_iterator(data_.get());
	}

	const_iterator end() const noexcept
	{
		return const_iterator(data_.get() + size_);
	}

	typename iterator::reference operator[](size_t index) noexcept
	{
		return data_[index];
	}

	typename const_iterator::reference operator[](size_t index) const noexcept
	{
		return data_.get()[index];
	}

	typename iterator::reference at(size_t index)
	{
		if (index >= size_)
		{
			throw std::out_of_range("Index out of range");
		}
		return data_[index];
	}

	typename const_iterator::reference at(size_t index) const
	{
		if (index >= size_)
		{
			throw std::out_of_range("Index out of range");
		}
		return data_[index];
	}

	size_t size() const noexcept { return size_; }

	size_t capacity() const noexcept { return capacity_; }

	void swap(simple_vector& other) noexcept
	{
		data_.swap(other.data_);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}

	friend void swap(simple_vector& lhs, simple_vector& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	void reserve(size_t new_cap)
	{
		if (new_cap > capacity_)
		{
			T* new_data = static_cast<T*>(operator new(sizeof(T) * (new_cap)));
			for (size_t i = 0; i < size_; i++)
			{
				new (&new_data[i]) T(std::move(data_[i]));
				data_[i].~T();
			}

			capacity_ = new_cap;
			data_ = array_ptr<T>(new_data);
		}
	}

	void resize(size_t new_size)
	{
		if (new_size > capacity_)
		{
			reserve(new_size);
		}

		if (new_size > size_)
		{
			for (size_t i = size_; i < new_size; i++)
			{
				new (data_.get() + i) T();
			}
		}
		if (new_size < size_)
		{
			for (size_t i = new_size; i < size_; i++)
			{
				data_[i].~T();
			}
		}
		size_ = new_size;
	}

	iterator insert(const_iterator where, T&& value)
	{
		size_t index = where - begin();

		if (size_ >= capacity_)
		{
			size_t new_cap;
			if (capacity_ == 0)
			{
				new_cap = 1;
			}
			else
			{
				new_cap = capacity_ * 2;
			}
			reserve(new_cap);
		}

		if (index < size_)
		{
			new (data_.get() + size_) T(std::move(data_[size_ - 1]));

			for (size_t i = size_ - 1; i > index; --i)
			{
				data_[i] = std::move(data_[i - 1]);
			}

			data_[index] = std::move(value);
		}
		else
		{
			data_[size_] = std::move(value);
		}

		++size_;
		return iterator(data_.get() + index);
	}

	iterator insert(const_iterator where, const T& value)
	{
		size_t index = where - begin();

		if (size_ >= capacity_)
		{
			if (capacity_ == 0)
			{
				reserve(1);
			}
			else
			{
				reserve(capacity_ * 2);
			}
		}

		if (index < size_)
		{
			new (data_.get() + size_) T(std::move(data_[size_ - 1]));

			for (size_t i = size_ - 1; i > index; --i)
			{
				data_[i] = std::move(data_[i - 1]);
			}

			data_[index] = value;
		}
		else
		{
			data_[size_] = value;
		}

		++size_;
		return iterator(data_.get() + index);
	}

	void push_back(T&& value) { insert(end(), std::move(value)); }

	void clear() noexcept
	{
		for (size_t i = 0; i < size_; i++)
		{
			data_[i].~T();
		}
		size_ = 0;
	}

	void push_back(const T& value) { insert(end(), value); }

	bool empty() const noexcept { return size_ == 0; }

	void pop_back() { erase(end() - 1); }

	friend bool operator==(const simple_vector& lhs, const simple_vector& rhs)
	{
		if (lhs.size_ != rhs.size_)
		{
			return false;
		}
		for (size_t i = 0; i < lhs.size_; i++)
		{
			if (lhs.data_[i] != rhs.data_[i])
			{
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const simple_vector& lhs, const simple_vector& rhs)
	{
		return !(lhs == rhs);
	}

	friend auto operator<=>(const simple_vector& lhs, const simple_vector& rhs)
	{
		for (size_t i = 0; i < std::min(lhs.size_, rhs.size_); i++)
		{
			if (lhs.data_[i] != rhs.data_[i])
			{
				return lhs.data_[i] <=> rhs.data_[i];
			}
		}
		return lhs.size_ <=> rhs.size_;
	}

	friend std::ostream& operator<<(std::ostream& os, const simple_vector& vec)
	{
		os << "[";
		for (size_t i = 0; i < vec.size_; i++)
		{
			if (i > 0)
				os << ", ";
			os << vec.data_[i];
		}
		os << "]";
		return os;
	}
	iterator erase(iterator where)
	{
		size_t index = where - begin();

		for (size_t i = index + 1; i < size_; i++)
		{
			data_[i - 1] = std::move(data_[i]);
		}

		data_[size_ - 1].~T();

		--size_;
		return iterator(data_.get() + index);
	}

	friend simple_vector operator+(const simple_vector& other1,
								   const simple_vector& other2)
	{
		auto result = other1.size() + other2.size();
		simple_vector new_vector;

		for (int i = 0; i < other1.size(); i++)
		{
			new_vector.push_back(other1[i]);
		}

		for (int i = 0; i < other2.size(); i++)
		{
			new_vector.push_back(other2[i]);
		}

		return new_vector;
	}

   private:
	static bool alphabet_compare(const simple_vector<T>& lhs,
								 const simple_vector<T>& rhs)
	{
		for (size_t i = 0; i < std::min(lhs.size_, rhs.size_); i++)
		{
			if (lhs.data_[i] != rhs.data_[i])
			{
				return lhs.data_[i] < rhs.data_[i];
			}
		}
		return lhs.size_ < rhs.size_;
	}
	array_ptr<T> data_;
	size_t size_ = 0;
	size_t capacity_ = 0;
};
}  // namespace bmstu
