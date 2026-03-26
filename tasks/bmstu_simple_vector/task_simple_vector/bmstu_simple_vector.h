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
		using const_reference = const T&;

		using difference_type = std::ptrdiff_t;

		iterator() = default;

		iterator(const iterator& other) = default;

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

		iterator& operator=(const iterator& other) = default;

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
			return iterator(ptr_ + n);
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

	~simple_vector() = default;

	simple_vector(std::initializer_list<T> init) noexcept
	{
		size_ = init.size();
		capacity_ = init.size();
		if (size_ > 0)
		{
			data_ = array_ptr<T>(size_);

			size_t i = 0;
			for (const auto& item : init)
			{
				data_[i] = item;
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
			data_ = array_ptr<T>(size_);

			for (int i = 0; i < size_; i++)
			{
				data_[i] = other.data_[i];
			}
		}
	}

	simple_vector(simple_vector&& other) noexcept
	{
		data_ = array_ptr<T>(nullptr);
		size_ = 0;
		capacity_ = 0;
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
		data_ = array_ptr<T>(size);
		size_ = size;
		capacity_ = size_;
		for (size_t i = 0; i < size; i++)
		{
			data_[i] = value;
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

	typename const_iterator::const_reference operator[](
		size_t index) const noexcept
	{
		return data_[index];
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
			array_ptr<T> new_data_(new_cap);
			for (size_t i = 0; i < size_; i++)
			{
				new_data_[i] = std::move(data_[i]);
			}

			data_.swap(new_data_);
			capacity_ = new_cap;
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

	void push_back(T&& value)
	{
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

		data_[size_] = std::move(value);
		++size_;
	}

	void clear() noexcept
	{
		for (size_t i = 0; i < size_; i++)
		{
			data_[i].~T();
		}
		size_ = 0;
	}

	void push_back(const T& value)
	{
		size_t new_cap;
		if (size_ >= capacity_)
		{
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

		data_[size_] = value;
		++size_;
	}

	bool empty() const noexcept { return size_ == 0; }

	void pop_back()
	{
		if (size_ > 0)
		{
			data_[size_ - 1].~T();
			--size_;
		}
	}

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
		size_t min_size = std::min(lhs.size(), rhs.size());

		for (size_t i = 0; i < min_size; i++)
		{
			if (lhs[i] < rhs[i])
			{
				return std::strong_ordering::less;
			}
			if (rhs[i] < lhs[i])
			{
				return std::strong_ordering::greater;
			}
		}
		if (lhs.size() < rhs.size())
		{
			return std::strong_ordering::less;
		}
		if (lhs.size() > rhs.size())
		{
			return std::strong_ordering::greater;
		}

		return std::strong_ordering::equal;
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

		data_[index].~T();
		for (size_t i = index + 1; i < size_; i++)
		{
			data_[i - 1] = std::move(data_[i]);
		}

		data_[size_ - 1].~T();

		--size_;
		return iterator(data_.get() + index);
	}

   private:
	static bool alphabet_compare(const simple_vector<T>& lhs,
								 const simple_vector<T>& rhs)
	{
		return false;
	}
	array_ptr<T> data_;
	size_t size_ = 0;
	size_t capacity_ = 0;
};
}  // namespace bmstu
