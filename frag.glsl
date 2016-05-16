#version 330

in vec3 fcolor;
in vec3 fnorm;
in vec3 fpos;
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

void main() {

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
            color = Kd*(fcolor*0.75+lightC*0.25)*diffuse*attenA + Ks*lightC*spec*attenS + Ka*fcolor*attenA;
        }else{
            color = Kd*(fcolor*0.75+lightC*0.25)*diffuse*attenA + Ks*specColor*spec + Ka*fcolor*attenA;
        }

        if(glow > 0){
            color.r=max(color.r,fcolor.r*glow);
            color.g=max(color.g,fcolor.g*glow);
            color.b=max(color.b,fcolor.b*glow);
        }

        color_out=vec4(color,1);

}
