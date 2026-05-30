#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <list>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "fast_streebog.h"

namespace bmstu
{

namespace detail
{

template <typename T, typename = void>
struct has_raw_bytes_impl : std::false_type
{
};

template <typename T>
struct has_raw_bytes_impl<
	T,
	std::void_t<decltype(std::declval<const T&>().rawBytes())>>
	: std::is_convertible<decltype(std::declval<const T&>().rawBytes()),
						  std::span<const uint8_t>>
{
};

template <typename T, typename = void>
struct has_equality_operator_impl : std::false_type
{
};

template <typename T>
struct has_equality_operator_impl<
	T,
	std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>>
	: std::is_convertible<decltype(std::declval<const T&>() ==
								   std::declval<const T&>()),
						  bool>
{
};

}  // namespace detail

template <typename T>
struct has_raw_bytes : detail::has_raw_bytes_impl<T>
{
};

template <typename T>
inline constexpr bool has_raw_bytes_v = has_raw_bytes<T>::value;

template <typename T>
struct has_equality_operator : detail::has_equality_operator_impl<T>
{
};

template <typename T>
inline constexpr bool has_equality_operator_v = has_equality_operator<T>::value;

template <typename K>
struct streebog_hash
{
	static_assert(has_raw_bytes_v<K> || std::is_trivially_copyable_v<K>,
				  "bmstu::streebog_hash: key type must be trivially-copyable "
				  "or provide `std::span<const uint8_t> rawBytes() const`.");

	std::size_t operator()(const K& key) const noexcept
	{
		uint8_t digest[32];
		if constexpr (has_raw_bytes_v<K>)
		{
			auto bytes = key.rawBytes();
			streebog_hash_256(bytes.data(), bytes.size(), digest);
		}
		else
		{
			streebog_hash_256(reinterpret_cast<const uint8_t*>(&key), sizeof(K),
							  digest);
		}
		std::size_t result = 0;
		std::memcpy(&result, digest, sizeof(std::size_t));
		return result;
	}
};

template <>
struct streebog_hash<std::string>
{
	std::size_t operator()(const std::string& key) const noexcept
	{
		uint8_t digest[32];
		streebog_hash_256(reinterpret_cast<const uint8_t*>(key.data()),
						  key.size(), digest);
		std::size_t result = 0;
		std::memcpy(&result, digest, sizeof(std::size_t));
		return result;
	}
};

template <typename K,
		  typename V,
		  typename Hash = streebog_hash<K>,
		  typename Equal = std::equal_to<K>>
class unordered_map
{
	static_assert(has_equality_operator_v<K>,
				  "bmstu::unordered_map: key type must provide `operator==`.");

   public:
	using size_type = size_t;
	using key_type = K;
	using mapped_type = V;
	using value_type = std::pair<const K, V>;

   private:
	static constexpr size_type DEFAULT_BUCKET_COUNT = 16;
	static constexpr double MAX_LOAD_FACTOR = 0.75;

	size_t bucket_for(const key_type& key) const
	{
		return hash_(key) % buckets_.size();
	}

	using bucket_type = std::list<value_type>;

	void rehash(size_t new_bucket_count)
	{
		std::vector<bucket_type> neww(new_bucket_count);
		for (auto& buckets : buckets_)
		{
			for (auto& pair : buckets)
			{
				size_type idx = hash_(pair.first) % new_bucket_count;
				neww[idx].push_back(std::move(pair));
			}
		}
		buckets_.swap(neww);
	}

	std::vector<bucket_type> buckets_;
	size_type size_;
	Hash hash_;
	Equal equal;

   public:
	explicit unordered_map(size_type bucket_count = DEFAULT_BUCKET_COUNT)
		: buckets_(bucket_count), size_(0)
	{
	}

	unordered_map(std::initializer_list<value_type> init)
		: buckets_(DEFAULT_BUCKET_COUNT), size_(0)
	{
		for (const auto& item : init)
		{
			insert(item);
		}
	}

	unordered_map(const unordered_map&) = default;
	unordered_map(unordered_map&& other) noexcept
		: buckets_(std::move(other.buckets_)),
		  size_(other.size_),
		  hash_(std::move(other.hash_)),
		  equal(std::move(other.equal))
	{
		other.size_ = 0;
		other.buckets_.clear();
		other.buckets_.resize(DEFAULT_BUCKET_COUNT);
	}

	unordered_map& operator=(const unordered_map& other)
	{
		if (this != &other)
		{
			unordered_map temp(other);
			swap(temp);
			return *this;
		}

		return *this;
	}

	unordered_map& operator=(unordered_map&&) = default;
	~unordered_map() = default;

	class iterator
	{
	   public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = unordered_map::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = std::ptrdiff_t;

		iterator() = default;

		iterator(std::vector<bucket_type>* buckets_ptr,
				 size_type bucket_index,
				 typename bucket_type::iterator list_it)
			: buckets_ptr_(buckets_ptr),
			  bucket_index_(bucket_index),
			  list_it_(list_it){};

		iterator(const iterator& other) = default;
		iterator(iterator&& other) noexcept = default;

		reference operator*() const { return *list_it_; }
		pointer operator->() const { return &(*list_it_); }

		iterator& operator=(const iterator& other) = default;
		iterator& operator=(iterator&& other) noexcept = default;

		iterator& operator++()
		{
			++list_it_;
			while (bucket_index_ < buckets_ptr_->size() &&
				   list_it_ == (*buckets_ptr_)[bucket_index_].end())
			{
				++bucket_index_;
				if (bucket_index_ < buckets_ptr_->size())
				{
					list_it_ = (*buckets_ptr_)[bucket_index_].begin();
				}
			}

			return *this;
		}

		iterator& operator--() = delete;

		iterator operator++(int)
		{
			iterator temp(*this);
			++(*this);
			return temp;
		}

		iterator& operator--(int) = delete;

		explicit operator bool() const { return buckets_ptr_ != nullptr; }

		bool operator==(const iterator& other) const
		{
			bool at_end =
				!buckets_ptr_ || bucket_index_ >= buckets_ptr_->size();
			bool other_at_end =
				!other.buckets_ptr_ ||
				other.bucket_index_ >= other.buckets_ptr_->size();

			if (at_end && other_at_end)
				return true;
			if (at_end != other_at_end)
				return false;
			return buckets_ptr_ == other.buckets_ptr_ &&
				   bucket_index_ == other.bucket_index_ &&
				   list_it_ == other.list_it_;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

	   private:
		std::vector<bucket_type>* buckets_ptr_ = nullptr;
		size_type bucket_index_ = 0;
		typename bucket_type::iterator list_it_;
	};

	class const_iterator
	{
	   public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = const typename unordered_map::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = std::ptrdiff_t;

		const_iterator() = default;

		const_iterator(const std::vector<bucket_type>* buckets_ptr,
					   size_type bucket_index,
					   typename bucket_type::const_iterator list_it)
			: buckets_ptr_(buckets_ptr),
			  bucket_index_(bucket_index),
			  list_it_(list_it)
		{
		}

		const_iterator(const iterator& it)
			: buckets_ptr_(it.buckets_ptr_),
			  bucket_index_(it.bucket_index_),
			  list_it_(it.list_it_)
		{
		}

		reference operator*() const { return *list_it_; }
		pointer operator->() const { return &(*list_it_); }

		const_iterator& operator++()
		{
			++list_it_;
			while (bucket_index_ < buckets_ptr_->size() &&
				   list_it_ == (*buckets_ptr_)[bucket_index_].end())
			{
				++bucket_index_;
				if (bucket_index_ < buckets_ptr_->size())
					list_it_ = (*buckets_ptr_)[bucket_index_].begin();
			}
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const const_iterator& o) const
		{
			bool at_end =
				!buckets_ptr_ || bucket_index_ >= buckets_ptr_->size();
			bool o_at_end =
				!o.buckets_ptr_ || o.bucket_index_ >= o.buckets_ptr_->size();

			if (at_end && o_at_end)
				return true;
			if (at_end != o_at_end)
				return false;
			return buckets_ptr_ == o.buckets_ptr_ &&
				   bucket_index_ == o.bucket_index_ && list_it_ == o.list_it_;
		}

		bool operator!=(const const_iterator& o) const { return !(*this == o); }

	   private:
		const std::vector<bucket_type>* buckets_ptr_ = nullptr;
		size_type bucket_index_ = 0;
		typename bucket_type::const_iterator list_it_;
	};

	double load_factor() const
	{
		return static_cast<double>(size_) /
			   static_cast<double>(buckets_.size());
	}

	size_type bucket_count() const { return buckets_.size(); }
	size_type size() const { return size_; }
	bool empty() const { return size_ == 0; }

	iterator begin()
	{
		for (size_t i = 0; i < buckets_.size(); i++)
		{
			if (!buckets_[i].empty())
			{
				return iterator(&buckets_, i, buckets_[i].begin());
			}
		}

		return end();
	}

	const_iterator begin() const { return cbegin(); }

	const_iterator cbegin() const
	{
		for (size_t i = 0; i < buckets_.size(); i++)
		{
			if (!buckets_[i].empty())
			{
				return const_iterator(&buckets_, i, buckets_[i].cbegin());
			}
		}

		return cend();
	}

	iterator end()
	{
		return iterator(&buckets_, buckets_.size(),
						typename bucket_type::iterator());
	}

	const_iterator end() const { return cend(); }

	const_iterator cend() const
	{
		return const_iterator(&buckets_, buckets_.size(),
							  typename bucket_type::const_iterator());
	}

	void clear()
	{
		for (auto& bucket : buckets_)
		{
			bucket.clear();
		}
		size_ = 0;
	}

	void swap(unordered_map& other)
	{
		std::swap(buckets_, other.buckets_);
		std::swap(size_, other.size_);
		std::swap(hash_, other.hash_);
	}

	iterator find(const key_type& key)
	{
		size_type idx = bucket_for(key);
		for (auto it = buckets_[idx].begin(); it != buckets_[idx].end(); it++)
		{
			if (equal(it->first, key))
			{
				return iterator(&buckets_, idx, it);
			}
		}

		return end();
	}

	const_iterator find(const key_type& key) const
	{
		size_type idx = bucket_for(key);
		for (auto it = buckets_[idx].begin(); it != buckets_[idx].end(); it++)
		{
			if (equal(it->first, key))
			{
				return const_iterator(&buckets_, idx, it);
			}
		}

		return cend();
	}

	bool contains(const key_type& key) const { return find(key) != cend(); }

	V& at(const key_type& key)
	{
		auto it = find(key);
		if (it == end())
		{
			throw std::out_of_range("Key not found");
		}
		return it->second;
	}

	const V& at(const key_type& key) const
	{
		auto it = find(key);
		if (it == end())
		{
			throw std::out_of_range("Key not found");
		}
		return it->second;
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		auto it = find(value.first);
		if (it != end())
		{
			return {it, false};
		}

		if (load_factor() > MAX_LOAD_FACTOR)
		{
			rehash(bucket_count() * 2);
		}

		size_type idx = bucket_for(value.first);
		buckets_[idx].push_front(value);
		++size_;
		return {iterator{&buckets_, idx, buckets_[idx].begin()}, true};
	}

	V& operator[](const key_type& key)
	{
		auto it = find(key);
		if (it != end())
		{
			return it->second;
		}
		else
		{
			if (load_factor() > MAX_LOAD_FACTOR)
			{
				rehash(bucket_count() * 2);
			}
			size_type idx = bucket_for(key);
			buckets_[idx].push_front({key, V()});
			++size_;
			auto last_it = buckets_[idx].begin();

			return last_it->second;
		}
	}

	bool erase(const key_type& key)
	{
		size_type idx = bucket_for(key);
		auto& bucket = buckets_[idx];
		for (auto it = bucket.begin(); it != bucket.end(); it++)
		{
			if (equal(it->first, key))
			{
				bucket.erase(it);
				--size_;
				return true;
			}
		}

		return false;
	}

	void reserve(size_type new_capacity)
	{
		size_type min_buckets = static_cast<size_t>(
			std::ceil(static_cast<double>(new_capacity)) / MAX_LOAD_FACTOR);

		if (min_buckets > bucket_count())
		{
			rehash(min_buckets);
		}
	}
	bool operator==(const unordered_map& other) const
	{
		if (size_ != other.size_)
			return false;

		for (const auto& pair : *this)
		{
			auto it = other.find(pair.first);
			if (it == other.cend() || !(it->second == pair.second))
				return false;
		}
		return true;
	}

	bool operator!=(const unordered_map& other) const
	{
		return !(*this == other);
	}
};

}  // namespace bmstu