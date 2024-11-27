#pragma once

#include <furi/core/mutex.h>
#include <memory>

template<typename T>
class Protected {
private:
    std::unique_ptr<T> model;
    FuriMutex* mutex;

public:
    class Guard {
    private:
        FuriMutex* mutex;
        T* model;
        
        Guard(FuriMutex* mutex, T* model) : mutex(mutex), model(model) {
            furi_mutex_acquire(mutex, FuriWaitForever);
        }
        
        friend class Protected;
        
    public:
        ~Guard() {
            furi_mutex_release(mutex);
        }
        
        T* operator->() { return model; }
        const T* operator->() const { return model; }
        T& operator*() { return *model; }
        const T& operator*() const { return *model; }
    };

    Protected() : model(std::make_unique<T>()), mutex(furi_mutex_alloc(FuriMutexTypeNormal)) {}
    
    ~Protected() {
        if (mutex) {
            furi_mutex_free(mutex);
        }
    }

    // Get protected access to the model
    Guard lock() {
        return Guard(mutex, model.get());
    }

    // Delete copy operations
    Protected(const Protected&) = delete;
    Protected& operator=(const Protected&) = delete;

    // Allow move operations
    Protected(Protected&& other) noexcept 
        : model(std::move(other.model)), mutex(other.mutex) {
        other.mutex = nullptr;
    }
    
    Protected& operator=(Protected&& other) noexcept {
        if (this != &other) {
            if (mutex) {
                furi_mutex_free(mutex);
            }
            model = std::move(other.model);
            mutex = other.mutex;
            other.mutex = nullptr;
        }
        return *this;
    }
}; 