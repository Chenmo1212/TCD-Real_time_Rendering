kernel BoxFilter : ImageComputationKernel<eComponentWise>
{
Image<eRead, eAccessRandom, eEdgeClamped> src;
Image<eWrite> dst;

param:
	int n;

local:
	int size;
	float nxn;

	void define() {
		defineParam(n, "box size", 3)
	}

	void init() {
		size = n;
		nxn = float(n * n);
	}

	void process(int2 pos) {
		float sum = 0;
		for (int i = 0; i <= size; i++) {
			for (int j = 0; j <= size; j++) {
				sum += src(pos.x + i, pos.y + j);
			}
		}
		dst() = sum / nxn;
	}
}
param:
int n;
local:
int halfSize; float nxn;
void define()
3); defineParam(nbox size"
void init()halfSize = n / 2; nxn = float(n * n);
void process(int2 pos) float sum = 0; for (int k = -halfSize; k <= halfSize; k++)for (int l = -halfSize; l <= halfSize; l++)sum += src(pos.x + 1, pos.y + k);
dst


kernel differenceKernel : ImageComputationKernel<eComponentWise>
{
	Image<eRead, eAccessPoint, eEdgeClamped> t0; // the input image t0
	Image<eRead, eAccessPoint, eEdgeClamped> t1; // the input image t1
	Image<eWrite> dst; // the output image
	void process() {
		dst() = t1() - t0();
	}
};