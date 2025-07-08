#pragma once
#include <vector>
#include <functional>

/**
 * @file Event.h
 * @brief Event listener system implemented using C++ templates.
 *
 * Provides a generic, flexible mechanism for event dispatching, allowing both free functions
 * and member methods to be registered and called when the event is triggered.
 *
 */

 /**
  * @brief Template-based Event class for managing function and method callbacks with parameters.
  *
  * This class allows listeners (free functions or object methods) to be registered and later
  * triggered with arguments of specified types. It supports:
  * - Free functions with or without parameters.
  * - Member methods with or without parameters.
  * 
  * It does not allow automatic deletion of object methods.
  * So you must remove the method from the listeners before destroying the object.
  *
  * @tparam Types Variadic template representing the argument types passed to the listeners.
  */
template <typename... Types>
class Event {

    /// Internal function wrapper type matching the event signature
    using Callback = std::function<void(Types...)>;

    /**
     * @brief Internal representation of a registered listener.
     */
    struct Listener {
        void* instancePtr;   ///< Pointer to the object instance (or nullptr for free functions)
        void* functionPtr;   ///< Raw pointer used for comparison and removal
        Callback callback;   ///< Callable that wraps the actual function/method
    };

    /// List of listenners
    std::vector<Listener> _listeners;

public:

    /**
     * @brief Add a free function with the exact signature void(Types...).
     * @param function Pointer to the function to be added.
     */
    void AddListener(void(*function)(Types...)) {
        _listeners.push_back({ nullptr, reinterpret_cast<void*>(function), function });
    }

    /**
     * @brief Add a free function with no parameters, ignoring the passed arguments.
     *
     * @param function Pointer to a function void().
     */
    void AddListener(void(*function)()) {
        _listeners.push_back({ nullptr, reinterpret_cast<void*>(function),
            [function](Types...) {
                function(); // arguments ignored
            }
            });
    }

    /**
     * @brief Remove a free function with signature void(Types...).
     * @param function Pointer to the function to be removed.
     */
    void RemoveListener(void(*function)(Types...)) {
        void* functionPtr = reinterpret_cast<void*>(function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [functionPtr](const Listener& listener) {
                return listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Remove a free function with signature void().
     * @param function Pointer to the function to be removed.
     */
    void RemoveListener(void(*function)()) {
        void* functionPtr = reinterpret_cast<void*>(function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [functionPtr](const Listener& listener) {
                return listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Add a member function with parameters.
     *
     * @tparam T Class type of the instance.
     * @param instance Pointer to the object.
     * @param function Pointer to the member method void(T::*)(Types...).
     */
    template<typename T>
    void AddListener(T* instance, void(T::* function)(Types...)) {
        _listeners.push_back({ instance, *reinterpret_cast<void**>(&function),
            [instance, function](Types... args) {
                (instance->*function)(args...);
            }
            });
    }

    /**
     * @brief Add a member function with no parameters, arguments are ignored.
     *
     * @tparam T Class type of the instance.
     * @param instance Pointer to the object.
     * @param function Pointer to the method void(T::*)().
     */
    template <typename T>
    void AddListener(T* instance, void(T::* function)()) {
        _listeners.push_back({ instance, *reinterpret_cast<void**>(&function),
            [instance, function](Types... args) {
                (instance->*function)();
            }
            });
    }

    /**
     * @brief Remove a member method with parameters.
     *
     * @tparam T Class type of the instance.
     * @param instance Pointer to the object.
     * @param function Pointer to the method void(T::*)(Types...).
     */
    template <typename T>
    void RemoveListener(T* instance, void(T::* function)(Types...)) {
        void* functionPtr = *reinterpret_cast<void**>(&function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [instance, functionPtr](const Listener& listener) {
                return listener.instancePtr == instance && listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Remove a member method with no parameters.
     *
     * @tparam T Class type of the instance.
     * @param instance Pointer to the object.
     * @param function Pointer to the method void(T::*)().
     */
    template <typename T>
    void RemoveListener(T* instance, void(T::* function)()) {
        void* functionPtr = *reinterpret_cast<void**>(&function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [instance, functionPtr](const Listener& listener) {
                return listener.instancePtr == instance && listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Removes all registered listeners.
     */
    void RemoveAllListeners() {
        _listeners.clear();
    }

    /**
     * @brief Trigger the event, invoking all registered callbacks.
     * @param args Arguments to forward to the listeners.
     */
    void Trigger(Types... args) const {
        for (const auto& listener : _listeners)
            listener.callback(args...);
    }
};

/**
 * @brief Specialization of Event for events with no arguments (Event<>).
 *
 * Simplifies the logic for functions and methods that do not take any parameters.
 * Necessary to avoid ambiguous overloads when Types... is empty.
 */
template <>
class Event<> {
    using Callback = std::function<void()>;


    struct Listener {
        void* instancePtr;
        void* functionPtr;
        Callback callback;
    };

    std::vector<Listener> _listeners;

public:
    /**
     * @brief Add a free function with no arguments.
     * @param function Pointer to the function.
     */
    void AddListener(void(*function)()) {
        _listeners.push_back({ nullptr, reinterpret_cast<void*>(function), function });
    }

    /**
     * @brief Remove a free function with no arguments.
     * @param function Pointer to the function.
     */
    void RemoveListener(void(*function)()) {
        void* functionPtr = reinterpret_cast<void*>(function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [functionPtr](const Listener& listener) {
                return listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Add a member method with no arguments.
     *
     * @tparam T Class type.
     * @param instance Pointer to the object.
     * @param function Pointer to the method.
     */
    template <typename T>
    void AddListener(T* instance, void(T::* function)()) {
        _listeners.push_back({ instance, *reinterpret_cast<void**>(&function),
            [instance, function]() {
                (instance->*function)();
            }
            });
    }

    /**
     * @brief Remove a member method with no arguments.
     *
     * @tparam T Class type.
     * @param instance Pointer to the object.
     * @param function Pointer to the method.
     */
    template <typename T>
    void RemoveListener(T* instance, void(T::* function)()) {
        void* functionPtr = *reinterpret_cast<void**>(&function);
        _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
            [instance, functionPtr](const Listener& listener) {
                return listener.instancePtr == instance && listener.functionPtr == functionPtr;
            }), _listeners.end());
    }

    /**
     * @brief Remove all listeners.
     */
    void RemoveAllListeners() {
        _listeners.clear();
    }

    /**
     * @brief Trigger the event, calling all listeners.
     */
    void Trigger() const {
        for (const auto& l : _listeners)
            l.callback();
    }
};
