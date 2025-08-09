import { Routes } from '@angular/router';
import { HomeComponent } from './home/home.component';
import { LeaderboardComponent } from './leaderboard/leaderboard.component';

const titleSuffix = 'Hitcon Badge Battle!';

export const routes: Routes = [
    {
        path: '',
        component: HomeComponent,
        data: { animations: 'homePage' },
        title: titleSuffix,
    },
    {
        path: 'rank',
        component: LeaderboardComponent,
        data: { animations: 'rankPath'},
        title: `Leaderboard | ${titleSuffix}`
    }
];
