# 4DGaussianSplatRendering
Repository for my bachelor thesis project.  
In this project i will look at different ways of rendering Gaussian splats from 2D all the way to 4D.
The most important part will then be the parameterization of 4D Gaussians and to find a way to make them move in a predefined way.

### Abstract
Gaussian splatting as a tool for rendering volumetric scenes has proven to be a suitable alternative to other common techniques such as ray tracing and ray marching. Until a short while ago, Gaussians, commonly used by various reconstructive methods to depict 3D scenes, were limited in their abilities, only allowing for static scene reconstruction. With the introduction of 4D Gaussians the depiction of a dynamic scene becomes possible allowing for motion and deformation of a scene over time. In this work the possibilities and limitations of 4D Gaussians are explored with respect to their capabilities of modeling different types of motion. Different possibilities for parameterizing these Gaussians will also be a major part, finding a way to define a path they are supposed to follow, showcasing the results in some appropriate demo scenes.

Example render of some 2D splat:  
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/2DSplatsOnScreen_01.png?raw=true" width="30%"></img> 

Example render of a 3D Gaussian from various angles:
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/3D_Splat_Angle_01.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/3D_Splat_Angle_02.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/3D_Splat_Angle_03.png?raw=true" width="23%"></img> 

Example of batch rendering test with 10,000,000 splats in a 400x400x400 cube:  
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/screenshot_05.png?raw=true" width="30%"></img> 

Its tea time!
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/UtahTeapot.png?raw=true" width="30%"></img> 

4D Gaussian can move through time
![4D Gaussian moving](https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Splat4DTimeMotion.gif?raw=true)

They can even move smoothly
![4D Gaussian moving smooth](https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Splat4DContinousMotion.gif?raw=true)

Motion of teapot arround a circel:
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_01.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_02.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_03.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_04.png?raw=true" width="23%"></img> 

Lower sampeling rate on the cirle:
<br><img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_LowRes_01.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_LowRes_02.png?raw=true" width="23%"></img> <img src="https://github.com/EndMy5uffering/4DGaussianSplatRendering/blob/main/Screenshots/Experiment_NonLinearMotion_LowRes_03.png?raw=true" width="23%"></img>  

Resources:  
[Introduction to 3D Gaussian splatting](https://huggingface.co/blog/gaussian-splatting)  
[EWA Splatting](https://www.cs.umd.edu/~zwicker/publications/EWASplatting-TVCG02.pdf)  
[4D Gaussian Splatting for Real-Time Dynamic Scene Rendering](https://guanjunwu.github.io/4dgs/)  
