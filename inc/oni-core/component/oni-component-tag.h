#pragma once

namespace oni {
    namespace component {
        /// Audio
        struct Tag_Audible {
        };

        /// Graphics
        struct Tag_Static {
        };

        struct Tag_Dynamic {
        };

        struct Tag_ColorShaded {
        };

        struct Tag_TextureShaded {
        };

        struct Tag_LeavesMark {
        };

        struct Tag_SplatOnDeath {
        };

        struct Tag_Particle {
        };

        /// Sim
        struct Tag_SimModeServer {
        };

        struct Tag_SimModeClient {
        };

        /// Network
        struct Tag_RequiresNetworkSync {
        };

        struct Tag_OnlyComponentUpdate {
        };
    }
}