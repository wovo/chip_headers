#include "header.hpp"


int main(){
    
   PMC->PMC_MCKR |= PMC_MCKR_CSS_MAIN_CLK;    
   PMC->PMC_MCKR &= ~PMC_MCKR_CSS_Msk;
   PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk;
    
      /* Switch to 3-20MHz Xtal oscillator */
  PMC->CKGR_MOR = /* CKGR_MOR_KEY_PASSWD | */ /* SYS_BOARD_OSCOUNT | */ CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL;

  while ( !(PMC->PMC_SR & PMC_SR_MOSCSELS) )
  {
  }
 	PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
  {
  }

  /* Initialize PLLA */
  // PMC->CKGR_PLLAR = SYS_BOARD_PLLAR;
  while ( !(PMC->PMC_SR & PMC_SR_LOCKA) )
  {
  }

  /* Switch to main clock */
  PMC->PMC_MCKR = ( PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
  while ( !(PMC->PMC_SR & PMC_SR_MCKRDY) )
  {
  }

  /* Switch to PLLA */
  PMC->PMC_MCKR = (uint32_t ) 12; // SYS_BOARD_MCKR;
  while ( !(PMC->PMC_SR & PMC_SR_MCKRDY) )
  {
  }
  
  /* Determine clock frequency according to clock register values */
  switch ( PMC->PMC_MCKR & PMC_MCKR_CSS_Msk )
  {
    case PMC_MCKR_CSS_SLOW_CLK: /* Slow clock */
      if (SUPC->SR & SUPC_SR_OSCSEL)
      {
        //SystemCoreClock = CHIP_FREQ_XTAL_32K;
      }
      else
      {
        //SystemCoreClock = CHIP_FREQ_SLCK_RC;
      }
    break;

    case PMC_MCKR_CSS_MAIN_CLK: /* Main clock */
      if (PMC->CKGR_MOR & CKGR_MOR_MOSCSEL)
      {
        //SystemCoreClock = CHIP_FREQ_XTAL_12M;
      }
      else
      {
        //SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;

        switch (PMC->CKGR_MOR & CKGR_MOR_MOSCRCF_Msk)
        {
          case CKGR_MOR_MOSCRCF_4_MHz:
          break;

          case CKGR_MOR_MOSCRCF_8_MHz:
            SystemCoreClock *= 2U;
          break;

          case CKGR_MOR_MOSCRCF_12_MHz:
            SystemCoreClock *= 3U;
          break;

          default:
          break;
        }
      }
    break;

    case PMC_MCKR_CSS_PLLA_CLK: /* PLLA clock */
    case PMC_MCKR_CSS_UPLL_CLK: /* UPLL clock */
      if ( PMC->CKGR_MOR & CKGR_MOR_MOSCSEL )
      {
        SystemCoreClock = CHIP_FREQ_XTAL_12M;
      }
      else
      {
        SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;

        switch ( PMC->CKGR_MOR & CKGR_MOR_MOSCRCF_Msk )
        {
          case CKGR_MOR_MOSCRCF_4_MHz:
          break;

          case CKGR_MOR_MOSCRCF_8_MHz:
            SystemCoreClock *= 2U;
          break;

          case CKGR_MOR_MOSCRCF_12_MHz:
            SystemCoreClock *= 3U;
          break;

          default:
          break;
        }
      }
      if ( (PMC->PMC_MCKR & PMC_MCKR_CSS_Msk) == PMC_MCKR_CSS_PLLA_CLK )
      {
        SystemCoreClock *= ((((PMC->CKGR_PLLAR) & CKGR_PLLAR_MULA_Msk) >> CKGR_PLLAR_MULA_Pos) + 1U);
        SystemCoreClock /= ((((PMC->CKGR_PLLAR) & CKGR_PLLAR_DIVA_Msk) >> CKGR_PLLAR_DIVA_Pos));
      }
      else
      {
        SystemCoreClock = CHIP_FREQ_UTMIPLL / 2U;
      }
    break;
  }

  if ( (PMC->PMC_MCKR & PMC_MCKR_PRES_Msk) == PMC_MCKR_PRES_CLK_3 )
  {
    SystemCoreClock /= 3U;
  }
  else
  {
    SystemCoreClock >>= ((PMC->PMC_MCKR & PMC_MCKR_PRES_Msk) >> PMC_MCKR_PRES_Pos);
  }
}

/**
 * Initialize flash.
 */
static void system_init_flash( uint32_t dw_clk )
{
  /* Set FWS for embedded Flash access according to operating frequency */
  if ( dw_clk < CHIP_FREQ_FWS_0 )
  {
    EFC0->EEFC_FMR = EEFC_FMR_FWS(0);
    EFC1->EEFC_FMR = EEFC_FMR_FWS(0);
  }
  else
  {
    if ( dw_clk < CHIP_FREQ_FWS_1 )
    {
      EFC0->EEFC_FMR = EEFC_FMR_FWS(1);
      EFC1->EEFC_FMR = EEFC_FMR_FWS(1);
    }
    else
    {
      if ( dw_clk < CHIP_FREQ_FWS_2 )
      {
        EFC0->EEFC_FMR = EEFC_FMR_FWS(2);
        EFC1->EEFC_FMR = EEFC_FMR_FWS(2);
      }
      else
      {
        if ( dw_clk < CHIP_FREQ_FWS_3 )
        {
          EFC0->EEFC_FMR = EEFC_FMR_FWS(3);
          EFC1->EEFC_FMR = EEFC_FMR_FWS(3);
        }
        else
        {
          EFC0->EEFC_FMR = EEFC_FMR_FWS(4);
          EFC1->EEFC_FMR = EEFC_FMR_FWS(4);
        }
      }
    }
  }  

   
}