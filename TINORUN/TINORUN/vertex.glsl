#version 330 core
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1
//--- in_TexCoord: attribute index 2 (�ؽ�ó ��ǥ)
//--- in_Normal: attribute index 3 (���� ����)

layout (location = 0) in vec3 in_Position; //--- ��ġ ����: attribute position 0
layout (location = 1) in vec3 in_Color; //--- �÷� ����: attribute position 1
layout (location = 2) in vec2 in_TexCoord; //--- �ؽ�ó ��ǥ: attribute position 2
layout (location = 3) in vec3 in_Normal; //--- ���� ����: attribute position 3

uniform mat4 uMVP; //--- MVP ���
uniform mat4 uModel; //--- �� ��� (���� ����)

out vec3 out_Color; //--- �����׸�Ʈ ���̴��� ���
out vec2 out_TexCoord; //--- �ؽ�ó ��ǥ ���
out vec3 FragPos; //--- ���� ������ ���� ��ġ
out vec3 Normal; //--- ���� ������ ���� ����

void main()
{
    // ���� ��ġ ��ȯ
    gl_Position = uMVP * vec4(in_Position, 1.0);

    // ���� ����� ���� ���� ���� ��ġ
    FragPos = vec3(uModel * vec4(in_Position, 1.0));

    // ���� ���͸� ���� �������� ��ȯ (��յ� �����ϸ� ����)
    Normal = mat3(transpose(inverse(uModel))) * in_Normal;

    // �⺻ ���
    out_Color = in_Color;
    out_TexCoord = in_TexCoord;
}