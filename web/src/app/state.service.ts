import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class StateService {
  public floor = 0;
  public scaler = 99;
  public scrollLeft: number | null = null;
  public scrollTop: number | null = null;
}
