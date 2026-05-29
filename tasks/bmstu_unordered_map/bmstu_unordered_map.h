#pragma once
#include <cstring>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>
#include "fast_streebog.h"

namespace bmstu
{
template <typename item>
struct equal_to
{
	bool operator()(const item& lhs, const item& rhs) const
	{
		return lhs == rhs;
	}
};

template <typename K>
struct hash
{
	size_t operator()(const K& key) const
	{
		uint8_t digest[32];
		streebog_hash_256(reinterpret_cast<const uint8_t*>(&key), sizeof(K),
						  digest);
		std::size_t result = 0;
		std::memcpy(&result, digest, sizeof(std::size_t));
		return result;
	}
};

template <>
struct hash<const char*>
{
	size_t operator()(const char* str) const
	{
		uint8_t digest[32];
		size_t string_size = strlen(str);
		streebog_hash_256(reinterpret_cast<const uint8_t*>(str), string_size,
						  digest);
		std::size_t result = 0;
		std::memcpy(&result, digest, sizeof(std::size_t));
		return result;
	}
};

template <>
struct hash<size_t>
{
	size_t operator()(const size_t& key) const { return key; }
};

template <>
struct hash<int>
{
	size_t operator()(const int& key) const { return static_cast<size_t>(key); }
};

template <>
struct hash<char>
{
	size_t operator()(const char& key) const
	{
		return static_cast<size_t>(key);
	}
};

template <>
struct hash<double>
{
	size_t operator()(const double& key) const
	{
		size_t result;
		std::memcpy(&result, &key, sizeof(double));
		return result;
	}
};

template <>
struct hash<std::string>
{
	size_t operator()(const std::string& key) const
	{
		uint8_t digest[32];
		streebog_hash_256(reinterpret_cast<const uint8_t*>(key.c_str()),
						  key.size(), digest);
		std::size_t result = 0;
		std::memcpy(&result, digest, sizeof(std::size_t));
		return result;
	}
};

template <typename K,
		  typename V,
		  typename Hash = hash<K>,
		  typename KeyEqual = equal_to<K>>
class unordered_map
{
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
		std::vector<bucket_type> next(new_bucket_count);
		for (auto& bkt : buckets_)
		{
			for (auto& pair : bkt)
			{
				size_type idx = hash_(pair.first) % new_bucket_count;
				next[idx].push_back(std::move(pair));
			}
		}
		buckets_ = std::move(next);
	}

	std::vector<bucket_type> buckets_;
	size_type size_;
	Hash hash_;
	KeyEqual equal_;

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
		  equal_(std::move(other.equal_))
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
					list_it_ = (*buckets_ptr_)[bucket_index_].begin();
			}
			return *this;
		}

		iterator& operator--() = delete;
		iterator operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		iterator operator--(int) = delete;

		explicit operator bool() const { return buckets_ptr_ != nullptr; }

		bool operator==(const iterator& o) const
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

		bool operator!=(const iterator& o) const { return !(*this == o); }

		iterator& operator=(std::nullptr_t) noexcept
		{
			buckets_ptr_ = nullptr;
			return *this;
		}

	   private:
		std::vector<bucket_type>* buckets_ptr_ = nullptr;
		size_type bucket_index_ = 0;
		typename bucket_type::iterator list_it_;

		friend class unordered_map;
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
		for (size_t i = 0; i < buckets_.size(); ++i)
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
		for (size_t i = 0; i < buckets_.size(); ++i)
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

	void swap(unordered_map& other) noexcept
	{
		std::swap(buckets_, other.buckets_);
		std::swap(size_, other.size_);
		std::swap(hash_, other.hash_);
		std::swap(equal_, other.equal_);
	}

	iterator find(const key_type& key)
	{
		size_type idx = bucket_for(key);
		for (auto it = buckets_[idx].begin(); it != buckets_[idx].end(); ++it)
		{
			if (equal_(it->first, key))
			{
				return iterator(&buckets_, idx, it);
			}
		}
		return end();
	}

	const_iterator find(const key_type& key) const
	{
		size_type idx = bucket_for(key);
		for (auto it = buckets_[idx].cbegin(); it != buckets_[idx].cend(); ++it)
		{
			if (equal_(it->first, key))
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
		if (it == cend())
		{
			throw std::out_of_range("Key not found");
		}
		return it->second;
	}

	iterator insert(const value_type& value)
	{
		auto it = find(value.first);
		if (it != end())
		{
			it->second = value.second;
			return it;
		}

		if (load_factor() > MAX_LOAD_FACTOR)
		{
			rehash(bucket_count() * 2);
		}
		size_type idx = bucket_for(value.first);
		buckets_[idx].push_front(value);
		++size_;
		return iterator(&buckets_, idx, buckets_[idx].begin());
	}

	iterator insert(const key_type& key, const mapped_type& value)
	{
		return insert(value_type(key, value));
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
			buckets_[idx].emplace_back(key, V());
			++size_;
			return buckets_[idx].back().second;
		}
	}

	bool erase(const key_type& key)
	{
		size_type idx = bucket_for(key);
		auto& bucket = buckets_[idx];
		for (auto it = bucket.begin(); it != bucket.end(); ++it)
		{
			if (equal_(it->first, key))
			{
				bucket.erase(it);
				--size_;
				return true;
			}
		}
		return false;
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