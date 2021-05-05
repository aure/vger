//  Copyright © 2021 Audulus LLC. All rights reserved.

import SwiftUI
import MetalKit
import vger

struct VgerView: NSViewRepresentable {

    var renderCallback : (OpaquePointer) -> Void

    class Coordinator {
        var renderer = Renderer(device: MTLCreateSystemDefaultDevice()!)
    }

    func makeCoordinator() -> Coordinator {
        return Coordinator()
    }

    func makeNSView(context: Context) -> some NSView {
        let metalView = MTKView(frame: CGRect(x: 0, y: 0, width: 1024, height: 768),
                                device: MTLCreateSystemDefaultDevice()!)
        metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0)
        metalView.delegate = context.coordinator.renderer
        context.coordinator.renderer.renderCallback = renderCallback
        return metalView
    }

    func updateNSView(_ nsView: NSViewType, context: Context) {
        context.coordinator.renderer.renderCallback = renderCallback
    }
}

struct VgerView_Previews: PreviewProvider {
    static var previews: some View {
        VgerView(renderCallback: { vger in
            vgerRenderText(vger, "hello world", SIMD4<Float>(repeating: 1), 0)
        })
    }
}
