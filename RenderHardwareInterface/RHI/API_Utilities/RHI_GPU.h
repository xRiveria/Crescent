#pragma once
#include <memory>
#include <string>

namespace Aurora
{
    enum class RHI_GPU_Type
    {
        Unknown,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    class RHI_GPU
    {
    public:
        RHI_GPU(const uint32_t gpuVendorID, const RHI_GPU_Type gpuType, const char* gpuName, const uint64_t gpuTotalMemory, void* internalData)
        {
            this->m_GPUName = gpuName;
            this->m_GPUMemory = gpuTotalMemory / 1024 / 1024; // Memory is usually retrieved in bytes. Hence, we shall divide it by 1000 2 times to store it in MB.
            this->m_GPUVendorID = gpuVendorID;
            this->m_GPUType = gpuType;
            this->m_InternalData = internalData;
        }

        const std::string GetVendor() { return DecodeVendorID(m_GPUVendorID); }
        const std::string& GetGPUName() const { return m_GPUName; }
        uint64_t GetGPUMemory() const { return m_GPUMemory; }
        void* GetInternalData() const { return m_InternalData; }

    private:
        std::string DecodeVendorID(uint32_t vendorID)
        {
            return "Yet to Decode";
        }

    private:
        std::string m_GPUName = "Unknown";
        uint32_t m_GPUVendorID = 0;
        uint64_t m_GPUMemory = 0;
        RHI_GPU_Type m_GPUType = RHI_GPU_Type::Unknown;
        void* m_InternalData = nullptr;
    };
}