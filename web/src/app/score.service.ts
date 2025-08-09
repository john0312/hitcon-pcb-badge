import { HttpClient } from '@angular/common/http';
import { inject, Injectable } from '@angular/core';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class ScoreService {
  private http = inject(HttpClient);
  getLeaderBoard(): Observable<leaderboard[]> {
    return this.http.get<leaderboard[]>('/api/scores');
  }
}

export interface leaderboard {
  name: string;
  uid: number;
  scores: {
    shake_badge: number;
    dino: number;
    snake: number;
    tetris: number;
    connect_sponsor: number;
    rectf: number;
  };
  total_score: number;
}