#pragma once
#include <wrl/client.h>
#include "RHI_Utilities.h"

using namespace Microsoft::WRL;

namespace Aurora
{
    struct RHI_Context
    {
        // Debugging
#ifdef _DEBUG
        bool m_DebuggingEnabled = true;
        bool m_MarkersEnabled = true;
        bool m_ProfilingEnabled = true;
#else
        bool m_DebuggingEnabled = false;
        bool m_MarkersEnabled = false;
        bool m_ProfilingEnabled = true;
#endif

        // Queues
        void* m_QueueGraphics = nullptr;
        void* m_QueueCompute = nullptr;
        void* m_QueueTransfer = nullptr;

        uint32_t m_QueueGraphicsIndex = 0;
        uint32_t m_QueueTransferIndex = 0;
        uint32_t m_QueueComputeIndex = 0;

        uint32_t GetQueueIndex(RHI_Queue_Type queueType)
        {
            switch (queueType)
            {
                case RHI_Queue_Type::RHI_Queue_Graphics:
                    return m_QueueGraphicsIndex;

                case RHI_Queue_Type::RHI_Queue_Compute:
                    return m_QueueComputeIndex;

                case RHI_Queue_Type::RHI_Queue_Transfer:
                    return m_QueueTransferIndex;
            }

            std::cout << "Invalid queue type. Did you make an error?\n";
            return 0;
        }

        // Device Limits
        static inline uint32_t m_Texture2D_Dimensions_Max = 16384; // As per DX11 D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION.
    };
}

