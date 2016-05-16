#version 330

in vec3 fcolor;
in vec3 fnorm;
in vec3 fpos;
in vec2 fuv;
out vec4 color_out;

uniform vec3 lightP;
uniform vec3 lightC;
uniform vec3 specColor;

uniform float alpha;
uniform float Kd;
uniform float Ks;
uniform float Ka;
uniform float attenuation;
uniform float attenuationS;
uniform float glow;

uniform int spot;
uniform vec3 spotDir;
uniform float gatten;

uniform sampler2D samp;

void main() {

        vec3 scolor=vec3(texture(samp,fuv));
        //color_out = color;

        //difuse
        vec3 norm=normalize(fnorm);

        vec3 dir=lightP-fpos;
        float dist=length(dir);
        vec3 L=dir/dist;

        float attenA=gatten;
        float attenS=gatten;

        if(spot==1){
            attenA=clamp(1/(dist*attenuation),0,1);
            attenS=clamp(1/(dist*attenuationS),0,1);

            float spotCos=dot(L,-spotDir);
            float satt=pow(spotCos,30);
            attenA*=satt*1.5;
            attenS*=satt*1.5;
            attenA*=gatten;
            attenS*=gatten;
        }

        float diffuse=max(0,dot(norm,L));

    //spec
        vec3 R=2*dot(norm,L)*norm - L;
        vec3 V=normalize(-fpos);
        float spec=pow(max(0,dot(R,V)),alpha);

        vec3 color;

        if(specColor.r==1 && specColor.g==1 && specColor.b==1){
            color = Kd*(scolor*0.75+lightC*0.25)*diffuse*attenA + Ks*lightC*spec*attenS + Ka*scolor*attenA;
        }else{
            color = Kd*(scolor*0.75+lightC*0.25)*diffuse*attenA + Ks*specColor*spec*attenS + Ka*scolor*attenA;
        }

        if(glow > 0){
            color.r=max(color.r,scolor.r*glow);
            color.g=max(color.g,scolor.g*glow);
            color.b=max(color.b,scolor.b*glow);
        }

        color_out=vec4(color,1);

}
