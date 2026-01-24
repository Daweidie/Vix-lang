#ifndef VCONVERT_HPP
#define VCONVERT_HPP
#include <string>
#include <cstdlib>
#include <typeinfo>
#include <stdexcept>
#include "vtypes.hpp"
namespace detail {
    inline void reverse_string(char* str, int len) {
        int start = 0;
        int end = len - 1;
        while (start < end) {
            char temp = str[start];
            str[start] = str[end];
            str[end] = temp;
            start++;
            end--;
        }
    }

    inline int int_to_string(long long num, char* str) {
        int i = 0;
        bool isNegative = false;
        
        if (num == 0) {
            str[i++] = '0';
            str[i] = '\0';
            return i;
        }
        
        if (num < 0) {
            isNegative = true;
            num = -num;
        }
        
        while (num != 0) {
            int rem = num % 10;
            str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
            num = num/10;
        }
        
        if (isNegative) {
            str[i++] = '-';
        }
        
        str[i] = '\0';
        reverse_string(str, i);
        return i;
    }
    
    inline int double_to_string(double num, char* str, int precision = 6) {
        long long intPart = (long long)num;
        double fracPart = num - intPart;
        if (fracPart < 0) fracPart = -fracPart;
        
        int i = 0;
        if (intPart == 0) {
            str[i++] = '0';
        } else {
            char intStr[32];
            int len = int_to_string(intPart, intStr);
            for (int j = 0; j < len; j++) {
                str[i++] = intStr[j];
            }
        }
        if (precision > 0) {
            str[i++] = '.';
            for (int j = 0; j < precision; j++) {
                fracPart *= 10;
                int digit = (int)fracPart;
                str[i++] = '0' + digit;
                fracPart -= digit;
            }
        }
        
        str[i] = '\0';
        return i;
    }
}

namespace vconvert {
    inline long long to_int(const vtypes::VString& s) { 
        return std::stoll(static_cast<const std::string&>(s)); 
    }
    
    inline long long to_int(const std::string& s) { 
        return std::stoll(s); 
    }
    
    inline long long to_int(long long v) { 
        return v; 
    }
    
    inline long long to_int(int v) { 
        return v; 
    }
    
    inline long long to_int(double v) { 
        return static_cast<long long>(v); 
    }
    inline long long to_int_rtti(const vtypes::VString& var) {
        return std::stoll(static_cast<const std::string&>(var));
    }
    
    inline long long to_int_rtti(double var) {
        return static_cast<long long>(var);
    }
    
    inline long long to_int_rtti(int var) {
        return static_cast<long long>(var);
    }
    
    template<typename T>
    inline long long to_int_rtti(const T& var) {
        const std::type_info& t = typeid(var);
        return static_cast<long long>(var);
    }
    inline double to_double(const vtypes::VString& s) { 
        return std::stod(static_cast<const std::string&>(s)); 
    }
    
    inline double to_double(const std::string& s) { 
        return std::stod(s); 
    }
    
    inline double to_double(double v) { 
        return v; 
    }
    
    inline double to_double(long long v) { 
        return static_cast<double>(v); 
    }
    
    inline double to_double(int v) { 
        return static_cast<double>(v); 
    }
    class Convertible {
    public:
        virtual ~Convertible() = default;
        virtual long long to_int() const = 0;
        virtual double to_double() const = 0;
        virtual vtypes::VString to_vstring() const = 0;
    };
    
    class IntConvertible : public Convertible {
    private:
        long long value;
    public:
        explicit IntConvertible(long long v) : value(v) {}
        
        long long to_int() const override {
            return value;
        }
        
        double to_double() const override {
            return static_cast<double>(value);
        }
        
        vtypes::VString to_vstring() const override {
            char buffer[32];
            detail::int_to_string(value, buffer);
            return vtypes::VString(buffer);
        }
    };
    
    class DoubleConvertible : public Convertible {
    private:
        double value;
    public:
        explicit DoubleConvertible(double v) : value(v) {}
        
        long long to_int() const override {
            return static_cast<long long>(value);
        }
        
        double to_double() const override {
            return value;
        }
        
        vtypes::VString to_vstring() const override {
            char buffer[32];
            detail::double_to_string(value, buffer);
            return vtypes::VString(buffer);
        }
    };
    
    class StringConvertible : public Convertible {
    private:
        vtypes::VString value;
    public:
        explicit StringConvertible(const vtypes::VString& v) : value(v) {}
        
        long long to_int() const override {
            try {
                return std::stoll(static_cast<const std::string&>(value));
            } catch (...) {
                return 0;
            }
        }
        
        double to_double() const override {
            try {
                return std::stod(static_cast<const std::string&>(value));
            } catch (...) {
                return 0.0;
            }
        }
        
        vtypes::VString to_vstring() const override {
            return value;
        }
    };
    
    // 优化to_vstring函数，使用自定义实现替代std::to_string以提高性能
    inline vtypes::VString to_vstring(const vtypes::VString& s) { 
        return s; 
    }
    
    inline vtypes::VString to_vstring(const std::string& s) { 
        return vtypes::VString(s); 
    }
    
    inline vtypes::VString to_vstring(long long v) { 
        char buffer[32];
        detail::int_to_string(v, buffer);
        return vtypes::VString(buffer);
    }
    
    inline vtypes::VString to_vstring(int v) { 
        char buffer[32];
        detail::int_to_string(v, buffer);
        return vtypes::VString(buffer);
    }
    
    inline vtypes::VString to_vstring(double v) { 
        char buffer[32];
        detail::double_to_string(v, buffer);
        return vtypes::VString(buffer);
    }
    
    inline vtypes::VString to_vstring(float v) { 
        char buffer[32];
        detail::double_to_string(v, buffer);
        return vtypes::VString(buffer);
    }
    
    template<typename T> 
    inline vtypes::VString to_vstring(const T& v) { 
        char buffer[32];
        detail::int_to_string(v, buffer);
        return vtypes::VString(buffer);
    }
}

#endif // VCONVERT_HPP