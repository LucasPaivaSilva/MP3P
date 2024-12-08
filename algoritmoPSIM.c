void SimulationStep(
		double t, double delt, double *in, double *out,
		 int *pnError, char * szErrorMsg,
		 void ** reserved_UserData, int reserved_ThreadIndex, void * reserved_AppPtr) 
{
    // Variáveis estáticas para manter o estado entre as chamadas
    static double V_anterior = 0.0;
    static double I_anterior = 0.0;
    static double P_anterior = 0.0;
    static double D_anterior = 0.367;    // Duty cycle inicial (valor conforme conversado em sala)
	static double disableVariableStep = 0;

    // Constantes de ajuste
    double k_AD = 0.001; // Constante de ajuste para delta D

    // Substituindo as funções de leitura pelos valores de entrada in[1] (tensão) e in[0] (corrente)
    double V_atual = in[1];  // Tensão atual do painel solar
    double I_atual = in[0];  // Corrente atual do painel solar

    // Calcular a potência atual
    double P_atual = V_atual * I_atual;

    // Calcular as variações de tensão, corrente e potência
    double deltaV = V_atual - V_anterior;
    double deltaI = I_atual - I_anterior;
    double deltaP = P_atual - P_anterior;
    double deltaD;

    // Se deltaV é diferente de 0
    if (deltaV != 0) {
        // Verificar a derivada da potência em relação à tensão (deltaP/deltaV)
        double derivada_potencia = deltaP / deltaV;
		double derivada_potencia_abs = fabs(derivada_potencia);
        // Se deltaP/deltaV == 0, estamos no ponto de máxima potência
        if (derivada_potencia  == 0) {
			// Cheguei a testar pra caso próximo a zero, na prática vi que as oscilaçoes n mudavam muito
			// Enquanto estou mantendo assim
            // Manter o duty cycle atual
            deltaD = 0;
			out[1] = 1;
			out[2] = 1;
        } 
        // Se deltaP/deltaV > 0, estamos à esquerda do MPP, diminuir o duty cycle para aumentar a tensão
        else if (derivada_potencia > 0) {
            // Diminuir o duty cycle para aumentar a tensão
			if ((derivada_potencia_abs<20) && (derivada_potencia_abs>2) && (disableVariableStep == 0)){
				deltaD = (-k_AD *10); //*fabs(derivada_potencia)*10; // - k_AD*fabs(derivada_potencia);
			}
            else{
				deltaD = -k_AD*0.1;
			}
			out[1] = 1;
			out[2] = 0;
        } 
        // Se deltaP/deltaV < 0, estamos à direita do MPP, aumentar o duty cycle para diminuir a tensão
        else {
            // Aumentar o duty cycle para diminuir a tensão
			if ((derivada_potencia_abs<20) && (derivada_potencia_abs>2) && (disableVariableStep == 0)){
				deltaD = (k_AD * 10); //fabs(derivada_potencia)*10; // + k_AD*fabs(derivada_potencia);
			}
			else{
				deltaD = k_AD*0.1;
			}
			out[1] = 1;
			out[2] = 0;
        }
		out[3] = derivada_potencia_abs;
		//out[3] = fabs(derivada_potencia);
    } else {
        // Se deltaV == 0, apenas ajusta com base em deltaI
        if (deltaI > 0) {
            // Estamos à direita do MPP, aumentar o duty cycle para diminuir a tensão
            deltaD = k_AD * deltaI;
        } else if (deltaI < 0) {
            // Estamos à esquerda do MPP, diminuir o duty cycle para aumentar a tensão
            deltaD = -k_AD * deltaI;
        } else {
            // Se deltaI == 0, estamos no MPP, não ajusta o duty cycle
            deltaD = 0;
        }
		out[1] = 0;
		out[2] = 1;
    }

    // Atualizar o duty cycle
    double duty_cycle = D_anterior + deltaD;

    // Limitar o duty cycle entre 0 e 1
    if (duty_cycle > 1.0) duty_cycle = 1.0;
    if (duty_cycle < 0.0) duty_cycle = 0.0;

    // Saída do duty cycle
    out[0] = duty_cycle;

    // Atualizar os valores anteriores para a próxima iteração
    V_anterior = V_atual;
    I_anterior = I_atual;
    P_anterior = P_atual;
    D_anterior = duty_cycle;
}


