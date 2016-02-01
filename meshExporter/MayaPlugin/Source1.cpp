MFnDependencyNode depNodeAnim(itJointAnim.item());
MPlug animPlug = depNodeAnim.findPlug("translateX");
MPlugArray animPlugArray;
animPlug.connectedTo(animPlugArray, false, true);

for (unsigned int i = 0; i < animPlugArray.length(); i++) {
	MObject layerObject = animPlugArray[i].node();
	if (layerObject.hasFn(MFn::kAnimLayer)) {
		anim.animHeader.nrOfLayers++;
		anim.animLayer.resize(anim.animHeader.nrOfLayers);
		anim.animLayer.back().nrOfFrames = 0;
		anim.animLayer.back().layerObject = layerObject;

		MFnDependencyNode layerDepNode(layerObject);
		MPlug layerPlug = layerDepNode.findPlug("foregroundWeight");
		MPlugArray layerPlugArray;
		layerPlug.connectedTo(layerPlugArray, false, true);

		bool addednrOfKeys = false;
		for (unsigned int j = 0; j < layerPlugArray.length(); j++) {
			MObject blendObject = layerPlugArray[j].node();
			if (blendObject.hasFn(MFn::kBlendNodeDoubleLinear)) {
				MFnDependencyNode blendDepNode(blendObject);
				MPlug blendPlug = blendDepNode.findPlug("inputA");	// Maybe should be inputB
				MPlugArray blendPlugArray;
				blendPlug.connectedTo(blendPlugArray, true, false);

				for (unsigned int k = 0; k < blendPlugArray.length(); k++) {
					MObject curveObject = blendPlugArray[k].node();
					if (curveObject.hasFn(MFn::kAnimCurve)) {
						MFnAnimCurve rootCurve(curveObject);

						if (rootCurve.numKeys() > 0 && addednrOfKeys == false) {
							for (unsigned int l = 0; l < rootCurve.numKeys(); l++) {
								time = rootCurve.time(l);
								anim.animLayer.back().nrOfFrames++;
								anim.animLayer.back().time.push_back(((float)time.value() / 60.0f) * playBackSpeed);
								anim.animLayer.back().key.push_back(((int)time.value()));
							}
							addednrOfKeys = true;
						}
					}
				}
			}
		}
	}
}